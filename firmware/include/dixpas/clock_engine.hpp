#pragma once

#include <stdint.h>

#include "dixpas/types.hpp"

namespace dixpas {

class ClockEngine {
 public:
  void set_tempo_bpm_x10(uint16_t tempo_bpm_x10);
  [[nodiscard]] uint16_t tempo_bpm_x10() const { return tempo_bpm_x10_; }

  void start() { transport_state_ = TransportState::Playing; }
  void stop() { transport_state_ = TransportState::Stopped; }
  [[nodiscard]] TransportState transport_state() const { return transport_state_; }

  [[nodiscard]] uint32_t internal_tick_interval_micros() const;
  [[nodiscard]] uint8_t external_clock_to_internal_ticks() const {
    return kInternalTicksPerMidiClock;
  }

 private:
  uint16_t tempo_bpm_x10_ = 1200;
  TransportState transport_state_ = TransportState::Stopped;
};

}  // namespace dixpas
