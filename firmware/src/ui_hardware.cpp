#include "dixpas/ui_hardware.hpp"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace dixpas {

#if defined(ARDUINO)
namespace {

constexpr uint8_t kEncoderPhaseAPin = 2;
constexpr uint8_t kEncoderPhaseBPin = 3;
constexpr uint8_t kEncoderButtonPin = 4;
constexpr uint8_t kShiftRegisterLoadPin = 9;
constexpr uint8_t kShiftRegisterDataPin = 12;
constexpr uint8_t kShiftRegisterClockPin = 13;

constexpr uint8_t kShiftRegisterCount = 5;
constexpr bool kButtonsAreActiveLow = true;
constexpr bool kEncoderIsActiveLow = true;

bool decode_active_level(bool raw_state, bool active_low) {
  return active_low ? !raw_state : raw_state;
}

bool read_chain_bit(const uint8_t* bytes, uint8_t logical_index) {
  const uint8_t byte_index = static_cast<uint8_t>(logical_index / 8U);
  const uint8_t bit_index = static_cast<uint8_t>(7U - (logical_index % 8U));
  return (bytes[byte_index] & (1U << bit_index)) != 0U;
}

void read_shift_register_bytes(uint8_t* bytes) {
  digitalWrite(kShiftRegisterLoadPin, LOW);
  delayMicroseconds(5);
  digitalWrite(kShiftRegisterLoadPin, HIGH);

  for (uint8_t index = 0; index < kShiftRegisterCount; ++index) {
    bytes[index] = shiftIn(kShiftRegisterDataPin, kShiftRegisterClockPin, MSBFIRST);
  }
}

}  // namespace
#endif

void UiHardware::begin() {
#if defined(ARDUINO)
  pinMode(kEncoderPhaseAPin, INPUT_PULLUP);
  pinMode(kEncoderPhaseBPin, INPUT_PULLUP);
  pinMode(kEncoderButtonPin, INPUT_PULLUP);

  pinMode(kShiftRegisterLoadPin, OUTPUT);
  pinMode(kShiftRegisterClockPin, OUTPUT);
  pinMode(kShiftRegisterDataPin, INPUT);

  digitalWrite(kShiftRegisterLoadPin, HIGH);
  digitalWrite(kShiftRegisterClockPin, LOW);
#endif
}

UiInputSnapshot UiHardware::read_snapshot() const {
  UiInputSnapshot snapshot{};

#if defined(ARDUINO)
  uint8_t bytes[kShiftRegisterCount]{};
  read_shift_register_bytes(bytes);

  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    snapshot.track_a_steps[index] =
        decode_active_level(read_chain_bit(bytes, index), kButtonsAreActiveLow);
    snapshot.track_b_steps[index] =
        decode_active_level(read_chain_bit(bytes, static_cast<uint8_t>(10U + index)),
                            kButtonsAreActiveLow);
    snapshot.row3_steps[index] =
        decode_active_level(read_chain_bit(bytes, static_cast<uint8_t>(20U + index)),
                            kButtonsAreActiveLow);
  }

  snapshot.play_button =
      decode_active_level(read_chain_bit(bytes, 30U), kButtonsAreActiveLow);
  snapshot.stop_button =
      decode_active_level(read_chain_bit(bytes, 31U), kButtonsAreActiveLow);
  snapshot.reset_button =
      decode_active_level(read_chain_bit(bytes, 32U), kButtonsAreActiveLow);
  snapshot.mode_button =
      decode_active_level(read_chain_bit(bytes, 33U), kButtonsAreActiveLow);
  snapshot.shift_button =
      decode_active_level(read_chain_bit(bytes, 34U), kButtonsAreActiveLow);

  snapshot.encoder_button =
      decode_active_level(digitalRead(kEncoderButtonPin) != LOW, kEncoderIsActiveLow);
  snapshot.encoder_phase_a =
      decode_active_level(digitalRead(kEncoderPhaseAPin) != LOW, kEncoderIsActiveLow);
  snapshot.encoder_phase_b =
      decode_active_level(digitalRead(kEncoderPhaseBPin) != LOW, kEncoderIsActiveLow);
#endif

  return snapshot;
}

}  // namespace dixpas
