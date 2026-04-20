#include "dixpas/music_scales.hpp"

namespace dixpas {

namespace {

constexpr uint8_t kScaleMajor[] = {0, 2, 4, 5, 7, 9, 11};
constexpr uint8_t kScaleMinor[] = {0, 2, 3, 5, 7, 8, 10};
constexpr uint8_t kScaleHarmonicMinor[] = {0, 2, 3, 5, 7, 8, 11};
constexpr uint8_t kScalePentatonicMajor[] = {0, 2, 4, 7, 9};
constexpr uint8_t kScalePentatonicMinor[] = {0, 3, 5, 7, 10};
constexpr uint8_t kScaleDorian[] = {0, 2, 3, 5, 7, 9, 10};
constexpr uint8_t kScalePhrygian[] = {0, 1, 3, 5, 7, 8, 10};
constexpr uint8_t kScaleLydian[] = {0, 2, 4, 6, 7, 9, 11};
constexpr uint8_t kScaleMixolydian[] = {0, 2, 4, 5, 7, 9, 10};
constexpr uint8_t kScaleLocrian[] = {0, 1, 3, 5, 6, 8, 10};
constexpr uint8_t kScaleWholeTone[] = {0, 2, 4, 6, 8, 10};
constexpr uint8_t kScaleChromatic[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

constexpr ScaleDefinition kScales[kScaleCount] = {
    {"Major", "Major", kScaleMajor,
     static_cast<uint8_t>(sizeof(kScaleMajor) / sizeof(kScaleMajor[0]))},
    {"Minor", "Minor", kScaleMinor,
     static_cast<uint8_t>(sizeof(kScaleMinor) / sizeof(kScaleMinor[0]))},
    {"Harmonic Minor", "Harm Min", kScaleHarmonicMinor,
     static_cast<uint8_t>(sizeof(kScaleHarmonicMinor) / sizeof(kScaleHarmonicMinor[0]))},
    {"Pentatonic Major", "Pent Maj", kScalePentatonicMajor,
     static_cast<uint8_t>(sizeof(kScalePentatonicMajor) / sizeof(kScalePentatonicMajor[0]))},
    {"Pentatonic Minor", "Pent Min", kScalePentatonicMinor,
     static_cast<uint8_t>(sizeof(kScalePentatonicMinor) / sizeof(kScalePentatonicMinor[0]))},
    {"Dorian", "Dorian", kScaleDorian,
     static_cast<uint8_t>(sizeof(kScaleDorian) / sizeof(kScaleDorian[0]))},
    {"Phrygian", "Phrygian", kScalePhrygian,
     static_cast<uint8_t>(sizeof(kScalePhrygian) / sizeof(kScalePhrygian[0]))},
    {"Lydian", "Lydian", kScaleLydian,
     static_cast<uint8_t>(sizeof(kScaleLydian) / sizeof(kScaleLydian[0]))},
    {"Mixolydian", "Mixolyd", kScaleMixolydian,
     static_cast<uint8_t>(sizeof(kScaleMixolydian) / sizeof(kScaleMixolydian[0]))},
    {"Locrian", "Locrian", kScaleLocrian,
     static_cast<uint8_t>(sizeof(kScaleLocrian) / sizeof(kScaleLocrian[0]))},
    {"Whole Tone", "WholeTon", kScaleWholeTone,
     static_cast<uint8_t>(sizeof(kScaleWholeTone) / sizeof(kScaleWholeTone[0]))},
    {"Chromatic", "Chrom", kScaleChromatic,
     static_cast<uint8_t>(sizeof(kScaleChromatic) / sizeof(kScaleChromatic[0]))},
};

constexpr const char* kNoteNames[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
};

uint8_t clamp_note(int16_t value) {
  if (value < 0) {
    return 0U;
  }
  if (value > 127) {
    return 127U;
  }
  return static_cast<uint8_t>(value);
}

}  // namespace

const ScaleDefinition& scale_definition(uint8_t scale_id) {
  return kScales[scale_id % kScaleCount];
}

const char* scale_name(uint8_t scale_id) {
  return scale_definition(scale_id).name;
}

const char* scale_short_name(uint8_t scale_id) {
  return scale_definition(scale_id).short_name;
}

const char* note_name(uint8_t note) {
  return kNoteNames[note % 12U];
}

uint8_t resolve_scale_note(uint8_t root_note, uint8_t scale_id, uint8_t degree,
                           int8_t octave_offset, uint8_t base_midi_note) {
  const ScaleDefinition& scale = scale_definition(scale_id);
  const uint8_t scale_degree = static_cast<uint8_t>(degree % scale.length);
  const uint8_t octave_group = static_cast<uint8_t>(degree / scale.length);
  const int16_t final_octave_offset =
      static_cast<int16_t>(octave_offset) + octave_group;

  const int16_t note = static_cast<int16_t>(base_midi_note) +
                       static_cast<int16_t>(root_note % 12U) +
                       static_cast<int16_t>(scale.intervals[scale_degree]) +
                       static_cast<int16_t>(final_octave_offset * 12);
  return clamp_note(note);
}

}  // namespace dixpas
