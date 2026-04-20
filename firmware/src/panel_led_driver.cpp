#include "dixpas/panel_led_driver.hpp"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace dixpas {

#if defined(ARDUINO)
namespace {

constexpr uint8_t kShiftRegisterLatchPin = 10;
constexpr uint8_t kShiftRegisterDataPin = 11;
constexpr uint8_t kShiftRegisterClockPin = 13;
constexpr uint8_t kLedShiftRegisterCount = 3;

void set_output_bit(uint8_t* bytes, uint8_t logical_index, bool value) {
  const uint8_t byte_index = static_cast<uint8_t>(logical_index / 8U);
  const uint8_t bit_index = static_cast<uint8_t>(7U - (logical_index % 8U));
  const uint8_t mask = static_cast<uint8_t>(1U << bit_index);

  if (value) {
    bytes[byte_index] = static_cast<uint8_t>(bytes[byte_index] | mask);
  } else {
    bytes[byte_index] = static_cast<uint8_t>(bytes[byte_index] & ~mask);
  }
}

void write_shift_register_bytes(const uint8_t* bytes) {
  digitalWrite(kShiftRegisterLatchPin, LOW);
  for (uint8_t index = 0; index < kLedShiftRegisterCount; ++index) {
    shiftOut(kShiftRegisterDataPin, kShiftRegisterClockPin, MSBFIRST, bytes[index]);
  }
  digitalWrite(kShiftRegisterLatchPin, HIGH);
}

}  // namespace
#endif

void PanelLedDriver::begin() {
#if defined(ARDUINO)
  pinMode(kShiftRegisterLatchPin, OUTPUT);
  pinMode(kShiftRegisterDataPin, OUTPUT);
  pinMode(kShiftRegisterClockPin, OUTPUT);
  digitalWrite(kShiftRegisterLatchPin, HIGH);
  digitalWrite(kShiftRegisterDataPin, LOW);
  digitalWrite(kShiftRegisterClockPin, LOW);
#endif
}

void PanelLedDriver::render(const App& app, const UiController& ui, bool blink_on,
                            PanelLedFrame& frame) const {
  frame = {};

  const ProjectState& project = app.project();
  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    frame.track_a[index] = project.track_a.steps[index].active;
    frame.track_b[index] = project.track_b.steps[index].active;
  }

  frame.track_a[ui.selected_step(TrackId::A)] = true;
  frame.track_b[ui.selected_step(TrackId::B)] = true;

  if (app.has_playhead_step(TrackId::A) && blink_on) {
    frame.track_a[app.playhead_step(TrackId::A)] = true;
  }
  if (app.has_playhead_step(TrackId::B) && blink_on) {
    frame.track_b[app.playhead_step(TrackId::B)] = true;
  }

  frame.system[0] = app.transport_state() == TransportState::Playing;
  frame.system[1] = app.clock_source() == ClockSource::ExternalMidi;
  frame.system[2] = ui.shift_held();
  frame.system[3] = ui.page() == UiPage::GlobalEdit;
}

void PanelLedDriver::write(const PanelLedFrame& frame) const {
#if defined(ARDUINO)
  uint8_t bytes[kLedShiftRegisterCount]{};

  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    set_output_bit(bytes, index, frame.track_a[index]);
    set_output_bit(bytes, static_cast<uint8_t>(10U + index), frame.track_b[index]);
  }

  for (uint8_t index = 0; index < kSystemLedCount; ++index) {
    set_output_bit(bytes, static_cast<uint8_t>(20U + index), frame.system[index]);
  }

  write_shift_register_bytes(bytes);
#else
  (void)frame;
#endif
}

}  // namespace dixpas
