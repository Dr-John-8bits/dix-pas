#pragma once

#include <stdint.h>

#include "dixpas/clock_engine.hpp"
#include "dixpas/fixed_queue.hpp"
#include "dixpas/gate_output_engine.hpp"
#include "dixpas/generative_engine.hpp"
#include "dixpas/midi_din_engine.hpp"
#include "dixpas/midi_din_input_engine.hpp"
#include "dixpas/sequencer_engine.hpp"
#include "dixpas/types.hpp"

namespace dixpas {

class App {
 public:
  App();

  void load_project(const ProjectState& project);
  void apply_project(const ProjectState& project, bool reset_playhead);
  [[nodiscard]] const ProjectState& project() const { return sequencer_.project(); }

  void seed_demo_project();
  void set_random_seed(uint32_t seed);
  bool load_generative_recipe(uint8_t slot, GenerativeRecipe& recipe) const;
  bool has_generative_slot(uint8_t slot) const;
  void apply_generative_slot(uint8_t slot, bool reset_playhead);
  void mutate_generative_slot(uint8_t slot, bool reset_playhead);
  void apply_generative_recipe(const GenerativeRecipe& recipe, bool reset_playhead);
  void mutate_from_generative_recipe(const GenerativeRecipe& recipe, bool reset_playhead);
  void set_clock_source(ClockSource source);
  [[nodiscard]] ClockSource clock_source() const { return clock_.clock_source(); }

  void start();
  void resume();
  void stop();
  void reset_playhead();
  void tick_internal();
  void receive_midi_byte(uint8_t byte);

  bool pop_midi_byte(uint8_t& byte) { return midi_.pop_byte(byte); }
  bool pop_routed_event(EngineEvent& event) { return monitor_queue_.pop(event); }
  [[nodiscard]] bool gate_state(TrackId track) const { return gates_.gate_state(track); }
  [[nodiscard]] bool has_playhead_step(TrackId track) const {
    return sequencer_.has_playhead_step(track);
  }
  [[nodiscard]] uint8_t playhead_step(TrackId track) const {
    return sequencer_.playhead_step(track);
  }
  [[nodiscard]] uint32_t current_tick() const { return sequencer_.current_tick(); }

  [[nodiscard]] ClockEngine& clock() { return clock_; }
  [[nodiscard]] const ClockEngine& clock() const { return clock_; }
  [[nodiscard]] TransportState transport_state() const {
    return sequencer_.transport_state();
  }

 private:
  static constexpr size_t kMonitorQueueCapacity = 64;

  ClockEngine clock_{};
  SequencerEngine sequencer_{};
  GenerativeEngine generator_{};
  GenerativeBank generative_bank_{};
  MidiDinEngine midi_{};
  MidiDinInputEngine midi_in_{};
  GateOutputEngine gates_{};
  FixedQueue<EngineEvent, kMonitorQueueCapacity> monitor_queue_{};

  static ProjectState build_default_project();
  void seed_demo_generative_bank();
  static GenerativeRecipe build_demo_recipe_dorian();
  static GenerativeRecipe build_demo_recipe_chromatic();
  void process_pending_engine_events();
  void process_pending_midi_input_events();
  void advance_from_external_clock();
};

}  // namespace dixpas
