#include "dixpas/sequencer_engine.hpp"

namespace dixpas {

namespace {

constexpr uint8_t kScaleMajor[] = {0, 2, 4, 5, 7, 9, 11};
constexpr uint8_t kScaleMinorNatural[] = {0, 2, 3, 5, 7, 8, 10};
constexpr uint8_t kScaleMinorPentatonic[] = {0, 3, 5, 7, 10};
constexpr uint8_t kScaleChromatic[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

struct ScaleView {
  const uint8_t* intervals = nullptr;
  uint8_t length = 0;

  constexpr ScaleView() = default;
  constexpr ScaleView(const uint8_t* intervals_in, uint8_t length_in)
      : intervals(intervals_in), length(length_in) {}
};

constexpr ScaleView kScales[] = {
    {kScaleMajor, static_cast<uint8_t>(sizeof(kScaleMajor) / sizeof(kScaleMajor[0]))},
    {kScaleMinorNatural,
     static_cast<uint8_t>(sizeof(kScaleMinorNatural) / sizeof(kScaleMinorNatural[0]))},
    {kScaleMinorPentatonic,
     static_cast<uint8_t>(sizeof(kScaleMinorPentatonic) / sizeof(kScaleMinorPentatonic[0]))},
    {kScaleChromatic, static_cast<uint8_t>(sizeof(kScaleChromatic) / sizeof(kScaleChromatic[0]))},
};

constexpr uint8_t kBaseMidiNote = 60;  // C4

uint8_t clamp_note(int16_t value) {
  if (value < 0) {
    return 0;
  }
  if (value > 127) {
    return 127;
  }
  return static_cast<uint8_t>(value);
}

}  // namespace

SequencerEngine::SequencerEngine() = default;

void SequencerEngine::load_project(const ProjectState& project) {
  project_ = project;
  project_.track_a.length = clamp_track_length(project_.track_a.length);
  project_.track_b.length = clamp_track_length(project_.track_b.length);
  project_.track_a.midi_channel = clamp_midi_channel(project_.track_a.midi_channel);
  project_.track_b.midi_channel = clamp_midi_channel(project_.track_b.midi_channel);

  for (Step& step : project_.track_a.steps) {
    step.probability = clamp_probability(step.probability);
    step.ratchet = clamp_ratchet(step.ratchet);
    step.gate = clamp_gate_percent(step.gate);
  }

  for (Step& step : project_.track_b.steps) {
    step.probability = clamp_probability(step.probability);
    step.ratchet = clamp_ratchet(step.ratchet);
    step.gate = clamp_gate_percent(step.gate);
  }

  reset_runtime_state();
}

void SequencerEngine::set_random_seed(uint32_t seed) {
  rng_state_ = seed == 0U ? 0xD1F00001U : seed;
}

void SequencerEngine::start() {
  output_queue_.clear();
  for (ScheduledEvent& scheduled_event : scheduled_events_) {
    scheduled_event.used = false;
  }

  current_tick_ = 0U;
  transport_state_ = TransportState::Playing;
  reset_runtime_state();
  process_step_boundary(current_tick_);
}

void SequencerEngine::resume() {
  transport_state_ = TransportState::Playing;
}

void SequencerEngine::stop() {
  force_all_notes_and_gates_off();
  transport_state_ = TransportState::Stopped;
}

void SequencerEngine::tick() {
  if (transport_state_ != TransportState::Playing) {
    return;
  }

  ++current_tick_;
  dispatch_due_events(current_tick_);

  if ((current_tick_ % kTicksPerStep) == 0U) {
    process_step_boundary(current_tick_);
  }
}

bool SequencerEngine::pop_event(EngineEvent& event) {
  return output_queue_.pop(event);
}

void SequencerEngine::reset_runtime_state() {
  track_a_runtime_ = {};
  track_b_runtime_ = {};
  chain_position_ = 0U;
  chain_pingpong_forward_ = true;

  const uint8_t track_a_length = clamp_track_length(project_.track_a.length);
  const uint8_t track_b_length = clamp_track_length(project_.track_b.length);
  const uint8_t chain_length = total_chain_length();

  if (project_.play_mode == PlayMode::Reverse) {
    track_a_runtime_.position = static_cast<uint8_t>(track_a_length - 1U);
    track_b_runtime_.position = static_cast<uint8_t>(track_b_length - 1U);
    chain_position_ = static_cast<uint8_t>(chain_length - 1U);
  }
}

void SequencerEngine::process_step_boundary(uint32_t tick) {
  if (project_.machine_mode == MachineMode::Chain20) {
    process_chain_step(tick);
    return;
  }

  process_dual_step(tick);
}

void SequencerEngine::process_dual_step(uint32_t tick) {
  const uint8_t track_a_length = clamp_track_length(project_.track_a.length);
  const uint8_t track_b_length = clamp_track_length(project_.track_b.length);

  process_track_step(TrackId::A, project_.track_a, track_a_runtime_.position, tick,
                     project_.track_a.midi_channel);
  process_track_step(TrackId::B, project_.track_b, track_b_runtime_.position, tick,
                     project_.track_b.midi_channel);

  advance_runtime_track(track_a_runtime_, track_a_length);
  advance_runtime_track(track_b_runtime_, track_b_length);
}

void SequencerEngine::process_chain_step(uint32_t tick) {
  const uint8_t track_a_length = clamp_track_length(project_.track_a.length);
  const uint8_t chain_length = total_chain_length();
  const uint8_t chain_channel = resolve_chain_midi_channel();

  if (chain_position_ < track_a_length) {
    process_track_step(TrackId::A, project_.track_a, chain_position_, tick, chain_channel);
  } else {
    const uint8_t step_index = static_cast<uint8_t>(chain_position_ - track_a_length);
    process_track_step(TrackId::B, project_.track_b, step_index, tick, chain_channel);
  }

  advance_chain_position(chain_length);
}

void SequencerEngine::process_track_step(TrackId track_id, const Track& track,
                                         uint8_t step_index, uint32_t tick,
                                         uint8_t midi_channel_override) {
  const Step& step = track.steps[step_index];
  if (!step.active || !passes_probability(step.probability)) {
    return;
  }

  const uint8_t midi_channel =
      midi_channel_override == 0U ? clamp_midi_channel(track.midi_channel) : midi_channel_override;
  const uint8_t note = resolve_note(track, step);
  const uint8_t ratchet = clamp_ratchet(step.ratchet);
  const uint8_t subdivision = step_subdivision_ticks(step);
  const uint8_t pulse_ticks = step_pulse_ticks(step);

  for (uint8_t repeat = 0; repeat < ratchet; ++repeat) {
    const uint32_t note_on_tick = tick + static_cast<uint32_t>(repeat) * subdivision;
    const uint32_t note_off_tick = note_on_tick + pulse_ticks;

    const EngineEvent gate_on(EventType::GateHigh, track_id, midi_channel, 0U, 0U, note_on_tick);
    const EngineEvent note_on(EventType::NoteOn, track_id, midi_channel, note, step.velocity,
                              note_on_tick);
    const EngineEvent note_off(EventType::NoteOff, track_id, midi_channel, note, 0U,
                               note_off_tick);
    const EngineEvent gate_off(EventType::GateLow, track_id, midi_channel, 0U, 0U,
                               note_off_tick);

    if (note_on_tick == current_tick_) {
      emit_event(gate_on);
      emit_event(note_on);
    } else {
      schedule_event(gate_on);
      schedule_event(note_on);
    }

    schedule_event(note_off);
    schedule_event(gate_off);
  }
}

void SequencerEngine::advance_runtime_track(RuntimeTrackState& runtime, uint8_t length) {
  switch (project_.play_mode) {
    case PlayMode::Forward:
      runtime.position = static_cast<uint8_t>((runtime.position + 1U) % length);
      break;
    case PlayMode::Reverse:
      runtime.position = runtime.position == 0U ? static_cast<uint8_t>(length - 1U)
                                                : static_cast<uint8_t>(runtime.position - 1U);
      break;
    case PlayMode::PingPong:
      if (length <= 1U) {
        runtime.position = 0U;
      } else if (runtime.pingpong_forward) {
        if (runtime.position + 1U >= length) {
          runtime.pingpong_forward = false;
          runtime.position = static_cast<uint8_t>(length - 2U);
        } else {
          runtime.position = static_cast<uint8_t>(runtime.position + 1U);
        }
      } else if (runtime.position == 0U) {
        runtime.pingpong_forward = true;
        runtime.position = 1U;
      } else {
        runtime.position = static_cast<uint8_t>(runtime.position - 1U);
      }
      break;
    case PlayMode::Random:
      runtime.position = next_random(length);
      break;
  }
}

void SequencerEngine::advance_chain_position(uint8_t total_length) {
  switch (project_.play_mode) {
    case PlayMode::Forward:
      chain_position_ = static_cast<uint8_t>((chain_position_ + 1U) % total_length);
      break;
    case PlayMode::Reverse:
      chain_position_ = chain_position_ == 0U ? static_cast<uint8_t>(total_length - 1U)
                                              : static_cast<uint8_t>(chain_position_ - 1U);
      break;
    case PlayMode::PingPong:
      if (total_length <= 1U) {
        chain_position_ = 0U;
      } else if (chain_pingpong_forward_) {
        if (chain_position_ + 1U >= total_length) {
          chain_pingpong_forward_ = false;
          chain_position_ = static_cast<uint8_t>(total_length - 2U);
        } else {
          chain_position_ = static_cast<uint8_t>(chain_position_ + 1U);
        }
      } else if (chain_position_ == 0U) {
        chain_pingpong_forward_ = true;
        chain_position_ = 1U;
      } else {
        chain_position_ = static_cast<uint8_t>(chain_position_ - 1U);
      }
      break;
    case PlayMode::Random:
      chain_position_ = next_random(total_length);
      break;
  }
}

void SequencerEngine::dispatch_due_events(uint32_t tick) {
  dispatch_due_events_for_type(tick, EventType::NoteOff);
  dispatch_due_events_for_type(tick, EventType::GateLow);
  dispatch_due_events_for_type(tick, EventType::GateHigh);
  dispatch_due_events_for_type(tick, EventType::NoteOn);
}

void SequencerEngine::dispatch_due_events_for_type(uint32_t tick, EventType type) {
  for (ScheduledEvent& scheduled_event : scheduled_events_) {
    if (!scheduled_event.used || scheduled_event.event.tick != tick || scheduled_event.event.type != type) {
      continue;
    }

    emit_event(scheduled_event.event);
    scheduled_event.used = false;
  }
}

void SequencerEngine::emit_event(const EngineEvent& event) {
  RuntimeTrackState& runtime = runtime_for_track(event.track);
  if (event.type == EventType::NoteOn) {
    runtime.note_active = true;
    runtime.active_note = event.note;
    runtime.active_channel = event.midi_channel;
  } else if (event.type == EventType::NoteOff) {
    if (runtime.note_active && runtime.active_note == event.note &&
        runtime.active_channel == event.midi_channel) {
      runtime.note_active = false;
    }
  } else if (event.type == EventType::GateHigh) {
    runtime.gate_high = true;
  } else if (event.type == EventType::GateLow) {
    runtime.gate_high = false;
  }

  output_queue_.push(event);
}

bool SequencerEngine::schedule_event(const EngineEvent& event) {
  for (ScheduledEvent& scheduled_event : scheduled_events_) {
    if (scheduled_event.used) {
      continue;
    }

    scheduled_event.used = true;
    scheduled_event.event = event;
    return true;
  }

  return false;
}

void SequencerEngine::force_all_notes_and_gates_off() {
  for (ScheduledEvent& scheduled_event : scheduled_events_) {
    scheduled_event.used = false;
  }

  force_track_off(TrackId::A, track_a_runtime_);
  force_track_off(TrackId::B, track_b_runtime_);
}

void SequencerEngine::force_track_off(TrackId track_id, RuntimeTrackState& runtime) {
  if (runtime.note_active) {
    emit_event(EngineEvent(EventType::NoteOff, track_id, runtime.active_channel,
                           runtime.active_note, 0U, current_tick_));
    runtime.note_active = false;
  }

  if (runtime.gate_high) {
    emit_event(EngineEvent(EventType::GateLow, track_id, runtime.active_channel, 0U, 0U,
                           current_tick_));
    runtime.gate_high = false;
  }
}

uint8_t SequencerEngine::next_random(uint8_t upper_bound) {
  rng_state_ ^= rng_state_ << 13U;
  rng_state_ ^= rng_state_ >> 17U;
  rng_state_ ^= rng_state_ << 5U;
  return upper_bound == 0U ? 0U : static_cast<uint8_t>(rng_state_ % upper_bound);
}

bool SequencerEngine::passes_probability(uint8_t probability) {
  if (probability >= 100U) {
    return true;
  }
  if (probability == 0U) {
    return false;
  }

  return next_random(100U) < probability;
}

uint8_t SequencerEngine::resolve_note(const Track& track, const Step& step) const {
  const ScaleView scale = kScales[project_.scale_id % (sizeof(kScales) / sizeof(kScales[0]))];
  const uint8_t scale_degree = static_cast<uint8_t>(step.degree % scale.length);
  const uint8_t octave_group = static_cast<uint8_t>(step.degree / scale.length);
  const int16_t octave_offset = static_cast<int16_t>(track.octave_offset) + octave_group;

  const int16_t note = static_cast<int16_t>(kBaseMidiNote) +
                       static_cast<int16_t>(project_.root_note % 12U) +
                       static_cast<int16_t>(scale.intervals[scale_degree]) +
                       static_cast<int16_t>(octave_offset * 12);
  return clamp_note(note);
}

uint8_t SequencerEngine::resolve_chain_midi_channel() const {
  return clamp_midi_channel(project_.track_a.midi_channel);
}

uint8_t SequencerEngine::total_chain_length() const {
  return static_cast<uint8_t>(clamp_track_length(project_.track_a.length) +
                              clamp_track_length(project_.track_b.length));
}

SequencerEngine::RuntimeTrackState& SequencerEngine::runtime_for_track(TrackId track_id) {
  return track_id == TrackId::A ? track_a_runtime_ : track_b_runtime_;
}

uint8_t SequencerEngine::step_pulse_ticks(const Step& step) {
  const uint8_t ratchet = clamp_ratchet(step.ratchet);
  const uint8_t subdivision = static_cast<uint8_t>(kTicksPerStep / ratchet);
  const uint8_t gate_percent = clamp_gate_percent(step.gate);

  uint8_t pulse_ticks =
      static_cast<uint8_t>((static_cast<uint16_t>(subdivision) * gate_percent + 99U) / 100U);
  if (pulse_ticks == 0U) {
    pulse_ticks = 1U;
  }

  if (ratchet > 1U && pulse_ticks >= subdivision) {
    pulse_ticks = static_cast<uint8_t>(subdivision - 1U);
  }

  return pulse_ticks;
}

uint8_t SequencerEngine::step_subdivision_ticks(const Step& step) {
  return static_cast<uint8_t>(kTicksPerStep / clamp_ratchet(step.ratchet));
}

}  // namespace dixpas
