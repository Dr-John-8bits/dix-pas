#pragma once

#include <stddef.h>
#include <stdint.h>

namespace dixpas {

constexpr size_t kStepsPerTrack = 10;
constexpr uint8_t kMinTrackLength = 1;
constexpr uint8_t kMaxTrackLength = 10;
constexpr uint8_t kMinProbability = 0;
constexpr uint8_t kMaxProbability = 100;
constexpr uint8_t kMinRatchet = 1;
constexpr uint8_t kMaxRatchet = 3;
constexpr uint8_t kMinGatePercent = 5;
constexpr uint8_t kMaxGatePercent = 100;
constexpr uint8_t kMinMidiChannel = 1;
constexpr uint8_t kMaxMidiChannel = 16;
constexpr uint8_t kInternalPpqn = 96;
constexpr uint8_t kMidiClockPpqn = 24;
constexpr uint8_t kInternalTicksPerMidiClock = kInternalPpqn / kMidiClockPpqn;
constexpr uint8_t kTicksPerStep = kInternalPpqn / 4;

enum class MachineMode : uint8_t {
  Dual = 0,
  Chain20 = 1,
};

enum class PlayMode : uint8_t {
  Forward = 0,
  Reverse = 1,
  PingPong = 2,
  Random = 3,
};

enum class TransportState : uint8_t {
  Stopped = 0,
  Playing = 1,
};

enum class ClockSource : uint8_t {
  Internal = 0,
  ExternalMidi = 1,
};

enum class TrackId : uint8_t {
  A = 0,
  B = 1,
};

enum class EventType : uint8_t {
  NoteOn = 0,
  NoteOff = 1,
  GateHigh = 2,
  GateLow = 3,
};

struct Step {
  bool active = false;
  uint8_t degree = 0;
  uint8_t probability = 100;
  uint8_t ratchet = 1;
  uint8_t gate = 75;
  uint8_t velocity = 100;

  constexpr Step() = default;
  constexpr Step(bool active_in, uint8_t degree_in, uint8_t probability_in,
                 uint8_t ratchet_in, uint8_t gate_in, uint8_t velocity_in)
      : active(active_in),
        degree(degree_in),
        probability(probability_in),
        ratchet(ratchet_in),
        gate(gate_in),
        velocity(velocity_in) {}
};

struct Track {
  Step steps[kStepsPerTrack]{};
  uint8_t length = kMaxTrackLength;
  uint8_t midi_channel = 1;
  int8_t octave_offset = 0;
};

struct ProjectState {
  Track track_a{};
  Track track_b{};
  MachineMode machine_mode = MachineMode::Dual;
  uint16_t tempo_bpm_x10 = 1200;
  uint8_t root_note = 0;
  uint8_t scale_id = 0;
  PlayMode play_mode = PlayMode::Forward;
};

struct EngineEvent {
  EventType type = EventType::NoteOn;
  TrackId track = TrackId::A;
  uint8_t midi_channel = 1;
  uint8_t note = 0;
  uint8_t velocity = 0;
  uint32_t tick = 0;

  constexpr EngineEvent() = default;
  constexpr EngineEvent(EventType type_in, TrackId track_in, uint8_t midi_channel_in,
                        uint8_t note_in, uint8_t velocity_in, uint32_t tick_in)
      : type(type_in),
        track(track_in),
        midi_channel(midi_channel_in),
        note(note_in),
        velocity(velocity_in),
        tick(tick_in) {}
};

constexpr uint8_t clamp_track_length(uint8_t value) {
  return value < kMinTrackLength ? kMinTrackLength
                                 : (value > kMaxTrackLength ? kMaxTrackLength
                                                            : value);
}

constexpr uint8_t clamp_probability(uint8_t value) {
  return value > kMaxProbability ? kMaxProbability : value;
}

constexpr uint8_t clamp_ratchet(uint8_t value) {
  return value < kMinRatchet ? kMinRatchet
                             : (value > kMaxRatchet ? kMaxRatchet : value);
}

constexpr uint8_t clamp_gate_percent(uint8_t value) {
  return value < kMinGatePercent ? kMinGatePercent
                                 : (value > kMaxGatePercent ? kMaxGatePercent
                                                            : value);
}

constexpr uint8_t clamp_midi_channel(uint8_t value) {
  return value < kMinMidiChannel ? kMinMidiChannel
                                 : (value > kMaxMidiChannel ? kMaxMidiChannel
                                                            : value);
}

}  // namespace dixpas
