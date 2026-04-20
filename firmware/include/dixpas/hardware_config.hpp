#pragma once

#include <stdint.h>

namespace dixpas::hardware {

constexpr uint8_t kMidiInRxPin = 0;
constexpr uint8_t kMidiOutTxPin = 1;

constexpr uint8_t kEncoderPhaseAPin = 2;
constexpr uint8_t kEncoderPhaseBPin = 3;
constexpr uint8_t kEncoderButtonPin = 4;

constexpr uint8_t kGateOutAPin = 5;
constexpr uint8_t kGateOutBPin = 6;

constexpr uint8_t kInputShiftRegisterLoadPin = 9;
constexpr uint8_t kLedShiftRegisterLatchPin = 10;
constexpr uint8_t kLedShiftRegisterDataPin = 11;
constexpr uint8_t kInputShiftRegisterDataPin = 12;
constexpr uint8_t kSharedShiftRegisterClockPin = 13;

constexpr uint8_t kI2cSdaPin = 18;
constexpr uint8_t kI2cSclPin = 19;

constexpr uint8_t kInputShiftRegisterCount = 5;
constexpr uint8_t kLedShiftRegisterCount = 3;

constexpr bool kButtonsAreActiveLow = true;
constexpr bool kEncoderIsActiveLow = true;

constexpr uint8_t kOledI2cAddress = 0x3CU;
constexpr uint8_t kFramI2cAddress = 0x50U;
constexpr uint32_t kI2cClockHz = 400000U;
constexpr uint8_t kOledTransferSize = 16U;
constexpr uint8_t kFramTransferSize = 24U;

constexpr uint16_t kBootSplashDurationMs = 1500U;

}  // namespace dixpas::hardware
