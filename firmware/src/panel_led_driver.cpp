#include "dixpas/panel_led_driver.hpp"

#include "dixpas/hardware_config.hpp"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace dixpas {

namespace {

bool hardware_test_uses_midi(HardwareTestMode mode) {
  return mode == HardwareTestMode::Combined || mode == HardwareTestMode::MidiOnly;
}

bool hardware_test_uses_gate(HardwareTestMode mode) {
  return mode == HardwareTestMode::Combined || mode == HardwareTestMode::GateOnly;
}

#if defined(ARDUINO)

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
  digitalWrite(hardware::kLedShiftRegisterLatchPin, LOW);
  for (uint8_t index = 0; index < hardware::kLedShiftRegisterCount; ++index) {
    shiftOut(hardware::kLedShiftRegisterDataPin, hardware::kSharedShiftRegisterClockPin,
             MSBFIRST, bytes[index]);
  }
  digitalWrite(hardware::kLedShiftRegisterLatchPin, HIGH);
}

#endif

}  // namespace

void PanelLedDriver::begin() {
#if defined(ARDUINO)
  pinMode(hardware::kLedShiftRegisterLatchPin, OUTPUT);
  pinMode(hardware::kLedShiftRegisterDataPin, OUTPUT);
  pinMode(hardware::kSharedShiftRegisterClockPin, OUTPUT);
  digitalWrite(hardware::kLedShiftRegisterLatchPin, HIGH);
  digitalWrite(hardware::kLedShiftRegisterDataPin, LOW);
  digitalWrite(hardware::kSharedShiftRegisterClockPin, LOW);
#endif
}

void PanelLedDriver::render(const App& app, const UiController& ui, bool blink_on,
                            PanelLedFrame& frame) const {
  frame = {};

  if (ui.page() == UiPage::Diagnostic) {
    for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
      frame.track_a[index] = true;
      frame.track_b[index] = blink_on;
    }

    frame.system[0] = ui.storage_available();
    frame.system[1] = ui.display_available();
    frame.system[2] = app.clock_source() == ClockSource::ExternalMidi;
    frame.system[3] = app.transport_state() == TransportState::Playing;
    return;
  }

  if (ui.page() == UiPage::HardwareTest) {
    const bool track_a_selected = ui.hardware_test_phase() < 2U;
    const bool phase_high = (ui.hardware_test_phase() % 2U) == 0U;
    const bool row_on = ui.hardware_test_running() && (phase_high || blink_on);

    for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
      frame.track_a[index] = track_a_selected && row_on;
      frame.track_b[index] = !track_a_selected && row_on;
    }

    frame.system[0] = ui.hardware_test_running();
    frame.system[1] = hardware_test_uses_midi(ui.hardware_test_mode());
    frame.system[2] = hardware_test_uses_gate(ui.hardware_test_mode());
    frame.system[3] = phase_high;
    return;
  }

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
  uint8_t bytes[hardware::kLedShiftRegisterCount]{};

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
