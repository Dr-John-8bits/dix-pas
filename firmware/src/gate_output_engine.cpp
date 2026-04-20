#include "dixpas/gate_output_engine.hpp"

namespace dixpas {

void GateOutputEngine::reset() {
  gate_a_high_ = false;
  gate_b_high_ = false;
}

void GateOutputEngine::set_gate(TrackId track, bool high) {
  if (track == TrackId::A) {
    gate_a_high_ = high;
    return;
  }

  gate_b_high_ = high;
}

bool GateOutputEngine::gate_state(TrackId track) const {
  return track == TrackId::A ? gate_a_high_ : gate_b_high_;
}

}  // namespace dixpas
