#pragma once

#include <stdint.h>

#include "dixpas/types.hpp"

namespace dixpas {

class GateOutputEngine {
 public:
  void reset();
  void set_gate(TrackId track, bool high);
  [[nodiscard]] bool gate_state(TrackId track) const;

 private:
  bool gate_a_high_ = false;
  bool gate_b_high_ = false;
};

}  // namespace dixpas
