#include "dixpas/generative_engine.hpp"

namespace dixpas {

namespace {

uint8_t clamp_percent(uint8_t value) {
  return value > 100U ? 100U : value;
}

}  // namespace

void GenerativeBank::clear() {
  for (uint8_t index = 0; index < kGenerativeSlotCount; ++index) {
    slots_[index] = {};
  }
}

bool GenerativeBank::store(uint8_t slot, const GenerativeRecipe& recipe) {
  if (slot >= kGenerativeSlotCount) {
    return false;
  }

  slots_[slot].occupied = true;
  slots_[slot].recipe = recipe;
  return true;
}

bool GenerativeBank::load(uint8_t slot, GenerativeRecipe& recipe) const {
  if (slot >= kGenerativeSlotCount || !slots_[slot].occupied) {
    return false;
  }

  recipe = slots_[slot].recipe;
  return true;
}

bool GenerativeBank::has_slot(uint8_t slot) const {
  return slot < kGenerativeSlotCount && slots_[slot].occupied;
}

void GenerativeEngine::set_seed(uint32_t seed) {
  rng_state_ = seed == 0U ? 0xA17ECAFEU : seed;
}

void GenerativeEngine::apply_recipe(ProjectState& project, const GenerativeRecipe& recipe) {
  set_seed(recipe.seed);
  project.root_note = recipe.root_note;
  project.scale_id = recipe.scale_id;
  project.machine_mode = recipe.machine_mode;
  project.play_mode = recipe.play_mode;

  if (recipe.track_a.enabled) {
    generate_project_track(project, TrackId::A, recipe.track_a.settings);
  }
  if (recipe.track_b.enabled) {
    generate_project_track(project, TrackId::B, recipe.track_b.settings);
  }
}

void GenerativeEngine::mutate_project(ProjectState& project, const GenerativeRecipe& recipe) {
  set_seed(recipe.seed ^ 0x5A17U);

  if (recipe.track_a.enabled && recipe.mutate_track_a_on_cycle) {
    mutate_track(project.track_a, recipe.track_a.settings);
  }
  if (recipe.track_b.enabled && recipe.mutate_track_b_on_cycle) {
    mutate_track(project.track_b, recipe.track_b.settings);
  }
}

void GenerativeEngine::generate_track(Track& track, const TrackGenerationSettings& settings) {
  const uint8_t length = track_length_from_settings(settings.rhythm);
  track.length = length;
  track.octave_offset = settings.melody.octave_offset;

  bool mask[kStepsPerTrack]{};
  build_euclidean_mask(settings.rhythm, mask, length);

  const uint8_t min_degree = settings.melody.min_degree <= settings.melody.max_degree
                                 ? settings.melody.min_degree
                                 : settings.melody.max_degree;
  uint8_t previous_degree = min_degree;
  bool pingpong_forward = true;
  uint8_t melodic_index = 0U;

  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    Step& step = track.steps[index];
    step.probability = clamp_probability(settings.melody.probability);
    step.ratchet = clamp_ratchet(settings.melody.ratchet);
    step.gate = clamp_gate_percent(settings.melody.gate);
    step.velocity = settings.melody.velocity;

    if (index >= length) {
      step.active = false;
      continue;
    }

    step.active = mask[index];
    if (!step.active && !settings.melody.write_inactive_steps) {
      continue;
    }

    const uint8_t degree = next_generated_degree(melodic_index, previous_degree,
                                                 pingpong_forward, settings.melody);
    step.degree = degree;
    previous_degree = degree;
    ++melodic_index;
  }
}

void GenerativeEngine::generate_project_track(ProjectState& project, TrackId track_id,
                                              const TrackGenerationSettings& settings) {
  Track& track = track_id == TrackId::A ? project.track_a : project.track_b;
  generate_track(track, settings);
}

void GenerativeEngine::mutate_track(Track& track, const TrackGenerationSettings& settings) {
  if (!settings.mutation.enabled) {
    return;
  }

  const uint8_t length = clamp_track_length(track.length);
  const uint8_t min_degree = settings.melody.min_degree <= settings.melody.max_degree
                                 ? settings.melody.min_degree
                                 : settings.melody.max_degree;
  const uint8_t max_degree = settings.melody.min_degree <= settings.melody.max_degree
                                 ? settings.melody.max_degree
                                 : settings.melody.min_degree;

  for (uint8_t index = 0; index < length; ++index) {
    Step& step = track.steps[index];
    if (!step.active) {
      continue;
    }

    if (chance(settings.mutation.chance_percent)) {
      const uint8_t span =
          static_cast<uint8_t>(settings.mutation.max_degree_delta * 2U + 1U);
      const int8_t delta = static_cast<int8_t>(next_bounded(span)) -
                           static_cast<int8_t>(settings.mutation.max_degree_delta);
      step.degree = wrapped_degree_step(step.degree, delta, min_degree, max_degree);
    }

    if (settings.mutation.probability_jitter > 0U &&
        chance(settings.mutation.chance_percent)) {
      const uint8_t span =
          static_cast<uint8_t>(settings.mutation.probability_jitter * 2U + 1U);
      const int16_t jitter =
          static_cast<int16_t>(next_bounded(span)) -
          static_cast<int16_t>(settings.mutation.probability_jitter);
      const int16_t probability = static_cast<int16_t>(step.probability) + jitter;
      step.probability = clamp_probability(
          probability < 0 ? 0U : static_cast<uint8_t>(probability));
    }
  }
}

uint32_t GenerativeEngine::next_random_u32() {
  rng_state_ ^= rng_state_ << 13U;
  rng_state_ ^= rng_state_ >> 17U;
  rng_state_ ^= rng_state_ << 5U;
  return rng_state_;
}

uint8_t GenerativeEngine::next_bounded(uint8_t upper_bound) {
  return upper_bound == 0U ? 0U : static_cast<uint8_t>(next_random_u32() % upper_bound);
}

bool GenerativeEngine::chance(uint8_t percent) {
  const uint8_t clamped = clamp_percent(percent);
  return clamped >= 100U ? true : next_bounded(100U) < clamped;
}

uint8_t GenerativeEngine::clamp_degree_in_range(int16_t value, uint8_t min_degree,
                                                uint8_t max_degree) {
  if (value < min_degree) {
    return min_degree;
  }
  if (value > max_degree) {
    return max_degree;
  }
  return static_cast<uint8_t>(value);
}

uint8_t GenerativeEngine::wrapped_degree_step(uint8_t value, int8_t delta, uint8_t min_degree,
                                              uint8_t max_degree) {
  const uint8_t range = static_cast<uint8_t>(max_degree - min_degree + 1U);
  int16_t offset = static_cast<int16_t>(value) - static_cast<int16_t>(min_degree) + delta;
  while (offset < 0) {
    offset += range;
  }
  while (offset >= range) {
    offset -= range;
  }
  return static_cast<uint8_t>(min_degree + offset);
}

uint8_t GenerativeEngine::track_length_from_settings(const EuclideanSettings& settings) {
  return clamp_track_length(settings.length == 0U ? kMaxTrackLength : settings.length);
}

void GenerativeEngine::build_euclidean_mask(const EuclideanSettings& settings, bool* mask,
                                            uint8_t length) {
  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    mask[index] = false;
  }

  const uint8_t pulses = settings.pulses > length ? length : settings.pulses;
  if (length == 0U || pulses == 0U) {
    return;
  }

  uint8_t base_pattern[kStepsPerTrack]{};
  uint8_t bucket = 0U;
  for (uint8_t index = 0; index < length; ++index) {
    bucket = static_cast<uint8_t>(bucket + pulses);
    if (bucket >= length) {
      bucket = static_cast<uint8_t>(bucket - length);
      base_pattern[index] = 1U;
    }
  }

  const uint8_t rotation = static_cast<uint8_t>(settings.rotation % length);
  for (uint8_t index = 0; index < length; ++index) {
    const uint8_t source_index =
        static_cast<uint8_t>((index + length - rotation) % length);
    mask[index] = base_pattern[source_index] != 0U;
  }
}

uint8_t GenerativeEngine::next_generated_degree(uint8_t sequence_index, uint8_t previous_degree,
                                                bool& pingpong_forward,
                                                const MelodyGenerationSettings& settings) {
  const uint8_t min_degree = settings.min_degree <= settings.max_degree
                                 ? settings.min_degree
                                 : settings.max_degree;
  const uint8_t max_degree = settings.min_degree <= settings.max_degree
                                 ? settings.max_degree
                                 : settings.min_degree;
  const uint8_t range = static_cast<uint8_t>(max_degree - min_degree + 1U);

  switch (settings.contour) {
    case MelodyContour::Ascending:
      return static_cast<uint8_t>(min_degree + (sequence_index % range));
    case MelodyContour::Descending:
      return static_cast<uint8_t>(max_degree - (sequence_index % range));
    case MelodyContour::PingPong: {
      if (range <= 1U) {
        return min_degree;
      }

      if (sequence_index == 0U) {
        pingpong_forward = true;
        return min_degree;
      }

      int16_t next = previous_degree;
      if (pingpong_forward) {
        ++next;
        if (next >= max_degree) {
          next = max_degree;
          pingpong_forward = false;
        }
      } else {
        --next;
        if (next <= min_degree) {
          next = min_degree;
          pingpong_forward = true;
        }
      }
      return clamp_degree_in_range(next, min_degree, max_degree);
    }
    case MelodyContour::Random:
      return static_cast<uint8_t>(min_degree + next_bounded(range));
    case MelodyContour::RandomWalk: {
      if (sequence_index == 0U) {
        return static_cast<uint8_t>(min_degree + (range / 2U));
      }

      const int8_t delta = static_cast<int8_t>(next_bounded(3U)) - 1;
      return wrapped_degree_step(previous_degree, delta, min_degree, max_degree);
    }
    case MelodyContour::Alternating: {
      const uint8_t low_index = static_cast<uint8_t>(sequence_index / 2U);
      if ((sequence_index % 2U) == 0U) {
        return static_cast<uint8_t>(min_degree + (low_index % range));
      }

      const uint8_t high_index = static_cast<uint8_t>(sequence_index / 2U);
      return static_cast<uint8_t>(max_degree - (high_index % range));
    }
  }

  return min_degree;
}

}  // namespace dixpas
