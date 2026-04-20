#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/fixed_queue.hpp"
#include "dixpas/types.hpp"

namespace dixpas {

class SequencerEngine {
 public:
  SequencerEngine();

  void load_project(const ProjectState& project);
  void apply_project(const ProjectState& project, bool reset_playhead);
  [[nodiscard]] const ProjectState& project() const { return project_; }

  void set_random_seed(uint32_t seed);

  void start();
  void resume();
  void stop();
  void reset_playhead();
  void tick();

  [[nodiscard]] TransportState transport_state() const { return transport_state_; }
  [[nodiscard]] uint32_t current_tick() const { return current_tick_; }
  [[nodiscard]] bool has_playhead_step(TrackId track_id) const;
  [[nodiscard]] uint8_t playhead_step(TrackId track_id) const;

  bool pop_event(EngineEvent& event);

 private:
  static constexpr size_t kOutputQueueCapacity = 64;
  static constexpr size_t kScheduledEventCapacity = 64;

  struct RuntimeTrackState {
    uint8_t position = 0;
    bool pingpong_forward = true;
    bool note_active = false;
    uint8_t active_note = 0;
    uint8_t active_channel = 1;
    bool gate_high = false;
    bool playhead_valid = false;
    uint8_t playhead_step = 0;
  };

  struct ScheduledEvent {
    bool used = false;
    EngineEvent event{};
  };

  ProjectState project_{};
  TransportState transport_state_ = TransportState::Stopped;
  uint32_t current_tick_ = 0;
  uint32_t rng_state_ = 0xD1F00001U;

  RuntimeTrackState track_a_runtime_{};
  RuntimeTrackState track_b_runtime_{};
  uint8_t chain_position_ = 0;
  bool chain_pingpong_forward_ = true;

  FixedQueue<EngineEvent, kOutputQueueCapacity> output_queue_{};
  ScheduledEvent scheduled_events_[kScheduledEventCapacity]{};

  void reset_runtime_state();
  void sanitize_project();
  void process_step_boundary(uint32_t tick);
  void process_dual_step(uint32_t tick);
  void process_chain_step(uint32_t tick);
  void process_track_step(TrackId track_id, const Track& track, uint8_t step_index,
                          uint32_t tick, uint8_t midi_channel_override = 0);

  void advance_runtime_track(RuntimeTrackState& runtime, uint8_t length);
  void advance_chain_position(uint8_t total_length);

  void dispatch_due_events(uint32_t tick);
  void dispatch_due_events_for_type(uint32_t tick, EventType type);
  void emit_event(const EngineEvent& event);
  bool schedule_event(const EngineEvent& event);

  void force_all_notes_and_gates_off();
  void force_track_off(TrackId track_id, RuntimeTrackState& runtime);

  [[nodiscard]] uint8_t next_random(uint8_t upper_bound);
  [[nodiscard]] bool passes_probability(uint8_t probability);
  [[nodiscard]] uint8_t resolve_note(const Track& track, const Step& step) const;
  [[nodiscard]] uint8_t resolve_chain_midi_channel() const;
  [[nodiscard]] uint8_t total_chain_length() const;
  [[nodiscard]] RuntimeTrackState& runtime_for_track(TrackId track_id);
  [[nodiscard]] static uint8_t step_pulse_ticks(const Step& step);
  [[nodiscard]] static uint8_t step_subdivision_ticks(const Step& step);
};

}  // namespace dixpas
