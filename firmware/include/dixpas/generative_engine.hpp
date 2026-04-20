#pragma once

#include <stdint.h>

#include "dixpas/types.hpp"

namespace dixpas {

enum class MelodyContour : uint8_t {
  Ascending = 0,
  Descending = 1,
  PingPong = 2,
  Random = 3,
  RandomWalk = 4,
  Alternating = 5,
};

struct EuclideanSettings {
  uint8_t length = kMaxTrackLength;
  uint8_t pulses = 4;
  uint8_t rotation = 0;
};

struct MelodyGenerationSettings {
  MelodyContour contour = MelodyContour::Ascending;
  uint8_t min_degree = 0;
  uint8_t max_degree = 7;
  int8_t octave_offset = 0;
  uint8_t probability = 100;
  uint8_t ratchet = 1;
  uint8_t gate = 75;
  uint8_t velocity = 100;
  bool write_inactive_steps = false;
};

struct MutationSettings {
  bool enabled = false;
  uint8_t chance_percent = 20;
  uint8_t max_degree_delta = 1;
  uint8_t probability_jitter = 0;
};

struct TrackGenerationSettings {
  EuclideanSettings rhythm{};
  MelodyGenerationSettings melody{};
  MutationSettings mutation{};
};

struct GenerativeTrackRecipe {
  bool enabled = true;
  TrackGenerationSettings settings{};
};

struct GenerativeRecipe {
  uint32_t seed = 0xA17ECAFEU;
  uint8_t root_note = 0;
  uint8_t scale_id = 0;
  MachineMode machine_mode = MachineMode::Dual;
  PlayMode play_mode = PlayMode::Forward;
  bool mutate_track_a_on_cycle = false;
  bool mutate_track_b_on_cycle = false;
  GenerativeTrackRecipe track_a{};
  GenerativeTrackRecipe track_b{};
};

constexpr uint8_t kGenerativeSlotCount = 4;

class GenerativeBank {
 public:
  void clear();
  bool store(uint8_t slot, const GenerativeRecipe& recipe);
  bool load(uint8_t slot, GenerativeRecipe& recipe) const;
  bool has_slot(uint8_t slot) const;

 private:
  struct Slot {
    bool occupied = false;
    GenerativeRecipe recipe{};
  };

  Slot slots_[kGenerativeSlotCount]{};
};

class GenerativeEngine {
 public:
  void set_seed(uint32_t seed);
  [[nodiscard]] uint32_t seed() const { return rng_state_; }

  void apply_recipe(ProjectState& project, const GenerativeRecipe& recipe);
  void mutate_project(ProjectState& project, const GenerativeRecipe& recipe);
  void generate_track(Track& track, const TrackGenerationSettings& settings);
  void generate_project_track(ProjectState& project, TrackId track_id,
                              const TrackGenerationSettings& settings);
  void mutate_track(Track& track, const TrackGenerationSettings& settings);

 private:
  uint32_t rng_state_ = 0xA17ECAFEU;

  [[nodiscard]] uint32_t next_random_u32();
  [[nodiscard]] uint8_t next_bounded(uint8_t upper_bound);
  [[nodiscard]] bool chance(uint8_t percent);

  static uint8_t clamp_degree_in_range(int16_t value, uint8_t min_degree, uint8_t max_degree);
  static uint8_t wrapped_degree_step(uint8_t value, int8_t delta, uint8_t min_degree,
                                     uint8_t max_degree);
  static uint8_t track_length_from_settings(const EuclideanSettings& settings);
  static void build_euclidean_mask(const EuclideanSettings& settings, bool* mask,
                                   uint8_t length);
  uint8_t next_generated_degree(uint8_t sequence_index, uint8_t previous_degree,
                                bool& pingpong_forward,
                                const MelodyGenerationSettings& settings);
};

}  // namespace dixpas
