#include "dixpas/ui_hardware.hpp"

#include "dixpas/hardware_config.hpp"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace dixpas {

#if defined(ARDUINO)
namespace {

bool decode_active_level(bool raw_state, bool active_low) {
  return active_low ? !raw_state : raw_state;
}

bool read_chain_bit(const uint8_t* bytes, uint8_t logical_index) {
  const uint8_t byte_index = static_cast<uint8_t>(logical_index / 8U);
  const uint8_t bit_index = static_cast<uint8_t>(7U - (logical_index % 8U));
  return (bytes[byte_index] & (1U << bit_index)) != 0U;
}

void read_shift_register_bytes(uint8_t* bytes) {
  digitalWrite(hardware::kInputShiftRegisterLoadPin, LOW);
  delayMicroseconds(5);
  digitalWrite(hardware::kInputShiftRegisterLoadPin, HIGH);

  for (uint8_t index = 0; index < hardware::kInputShiftRegisterCount; ++index) {
    bytes[index] = shiftIn(hardware::kInputShiftRegisterDataPin,
                           hardware::kSharedShiftRegisterClockPin, MSBFIRST);
  }
}

}  // namespace
#endif

void UiHardware::begin() {
#if defined(ARDUINO)
  pinMode(hardware::kEncoderPhaseAPin, INPUT_PULLUP);
  pinMode(hardware::kEncoderPhaseBPin, INPUT_PULLUP);
  pinMode(hardware::kEncoderButtonPin, INPUT_PULLUP);

  pinMode(hardware::kInputShiftRegisterLoadPin, OUTPUT);
  pinMode(hardware::kSharedShiftRegisterClockPin, OUTPUT);
  pinMode(hardware::kInputShiftRegisterDataPin, INPUT);

  digitalWrite(hardware::kInputShiftRegisterLoadPin, HIGH);
  digitalWrite(hardware::kSharedShiftRegisterClockPin, LOW);
#endif
}

UiInputSnapshot UiHardware::read_snapshot() const {
  UiInputSnapshot snapshot{};

#if defined(ARDUINO)
  uint8_t bytes[hardware::kInputShiftRegisterCount]{};
  read_shift_register_bytes(bytes);

  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    snapshot.track_a_steps[index] =
        decode_active_level(read_chain_bit(bytes, index), hardware::kButtonsAreActiveLow);
    snapshot.track_b_steps[index] =
        decode_active_level(read_chain_bit(bytes, static_cast<uint8_t>(10U + index)),
                            hardware::kButtonsAreActiveLow);
    snapshot.row3_steps[index] =
        decode_active_level(read_chain_bit(bytes, static_cast<uint8_t>(20U + index)),
                            hardware::kButtonsAreActiveLow);
  }

  snapshot.play_button =
      decode_active_level(read_chain_bit(bytes, 30U), hardware::kButtonsAreActiveLow);
  snapshot.stop_button =
      decode_active_level(read_chain_bit(bytes, 31U), hardware::kButtonsAreActiveLow);
  snapshot.reset_button =
      decode_active_level(read_chain_bit(bytes, 32U), hardware::kButtonsAreActiveLow);
  snapshot.mode_button =
      decode_active_level(read_chain_bit(bytes, 33U), hardware::kButtonsAreActiveLow);
  snapshot.shift_button =
      decode_active_level(read_chain_bit(bytes, 34U), hardware::kButtonsAreActiveLow);

  snapshot.encoder_button =
      decode_active_level(digitalRead(hardware::kEncoderButtonPin) != LOW,
                          hardware::kEncoderIsActiveLow);
  snapshot.encoder_phase_a =
      decode_active_level(digitalRead(hardware::kEncoderPhaseAPin) != LOW,
                          hardware::kEncoderIsActiveLow);
  snapshot.encoder_phase_b =
      decode_active_level(digitalRead(hardware::kEncoderPhaseBPin) != LOW,
                          hardware::kEncoderIsActiveLow);
#endif

  return snapshot;
}

}  // namespace dixpas
