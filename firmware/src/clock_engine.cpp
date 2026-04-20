#include "dixpas/clock_engine.hpp"

namespace dixpas {

void ClockEngine::set_tempo_bpm_x10(uint16_t tempo_bpm_x10) {
  tempo_bpm_x10_ = tempo_bpm_x10 == 0U ? 1200U : tempo_bpm_x10;
}

uint32_t ClockEngine::internal_tick_interval_micros() const {
  constexpr uint32_t kMicrosPerMinuteTimesTen = 600000000U;
  return kMicrosPerMinuteTimesTen / (tempo_bpm_x10_ * kInternalPpqn);
}

}  // namespace dixpas
