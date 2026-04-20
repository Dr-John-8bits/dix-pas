#pragma once

#include <stdint.h>

#include "dixpas/clock_engine.hpp"
#include "dixpas/sequencer_engine.hpp"
#include "dixpas/types.hpp"

namespace dixpas {

class App {
 public:
  App();

  void load_project(const ProjectState& project);
  [[nodiscard]] const ProjectState& project() const { return sequencer_.project(); }

  void seed_demo_project();
  void set_random_seed(uint32_t seed);

  void start();
  void stop();
  void tick();

  bool pop_event(EngineEvent& event) { return sequencer_.pop_event(event); }

  [[nodiscard]] ClockEngine& clock() { return clock_; }
  [[nodiscard]] const ClockEngine& clock() const { return clock_; }
  [[nodiscard]] TransportState transport_state() const {
    return sequencer_.transport_state();
  }

 private:
  ClockEngine clock_{};
  SequencerEngine sequencer_{};

  static ProjectState build_default_project();
};

}  // namespace dixpas
