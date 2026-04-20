#pragma once

#include <stdint.h>

namespace dixpas {

struct ScaleDefinition {
  const char* name = nullptr;
  const char* short_name = nullptr;
  const uint8_t* intervals = nullptr;
  uint8_t length = 0;

  constexpr ScaleDefinition() = default;
  constexpr ScaleDefinition(const char* name_in, const char* short_name_in,
                            const uint8_t* intervals_in, uint8_t length_in)
      : name(name_in),
        short_name(short_name_in),
        intervals(intervals_in),
        length(length_in) {}
};

constexpr uint8_t kScaleCount = 12;
constexpr uint8_t kDefaultBaseMidiNote = 60;

[[nodiscard]] const ScaleDefinition& scale_definition(uint8_t scale_id);
[[nodiscard]] const char* scale_name(uint8_t scale_id);
[[nodiscard]] const char* scale_short_name(uint8_t scale_id);
[[nodiscard]] const char* note_name(uint8_t note);
[[nodiscard]] uint8_t resolve_scale_note(uint8_t root_note, uint8_t scale_id, uint8_t degree,
                                         int8_t octave_offset,
                                         uint8_t base_midi_note = kDefaultBaseMidiNote);

}  // namespace dixpas
