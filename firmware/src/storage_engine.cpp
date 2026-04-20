#include "dixpas/storage_engine.hpp"

#include <string.h>

namespace dixpas {

namespace {

constexpr char kPresetMagic[4] = {'D', 'X', 'P', '1'};
constexpr char kMetadataMagic[4] = {'D', 'X', 'P', 'M'};
constexpr uint8_t kStepActiveFlag = 0x01U;

static_assert(sizeof(PresetSlotHeaderV1) + sizeof(PresetPayloadV1) <= kStoragePresetSlotSize,
              "Preset payload must fit in a fixed slot");

}  // namespace

MemoryStorageBackend::MemoryStorageBackend() {
  clear();
}

bool MemoryStorageBackend::read(uint16_t address, void* dst, size_t size) {
  if ((static_cast<size_t>(address) + size) > kMemoryStorageCapacity) {
    return false;
  }

  memcpy(dst, bytes_ + address, size);
  return true;
}

bool MemoryStorageBackend::write(uint16_t address, const void* src, size_t size) {
  if ((static_cast<size_t>(address) + size) > kMemoryStorageCapacity) {
    return false;
  }

  memcpy(bytes_ + address, src, size);
  return true;
}

void MemoryStorageBackend::clear(uint8_t fill) {
  memset(bytes_, fill, sizeof(bytes_));
}

StorageStatus StorageEngine::save_preset(uint8_t slot, const ProjectState& project) {
  if (slot >= kStoragePresetSlotCount) {
    return StorageStatus::InvalidSlot;
  }

  PresetPayloadV1 payload{};
  fill_payload_from_project(project, payload);

  PresetSlotHeaderV1 header{};
  memcpy(header.magic, kPresetMagic, sizeof(header.magic));
  header.format_version = kStorageFormatVersion;
  header.slot_index = slot;
  header.payload_size = static_cast<uint16_t>(sizeof(PresetPayloadV1));
  header.crc16 = crc16_ccitt(reinterpret_cast<const uint8_t*>(&payload), sizeof(payload));
  header.flags = 0x01U;

  const uint16_t address = slot_address(slot);
  if (!backend_.write(address, &header, sizeof(header))) {
    return StorageStatus::IoError;
  }
  if (!backend_.write(static_cast<uint16_t>(address + sizeof(header)), &payload, sizeof(payload))) {
    return StorageStatus::IoError;
  }

  return StorageStatus::Ok;
}

StorageStatus StorageEngine::load_preset(uint8_t slot, ProjectState& project) {
  if (slot >= kStoragePresetSlotCount) {
    return StorageStatus::InvalidSlot;
  }

  PresetSlotHeaderV1 header{};
  PresetPayloadV1 payload{};
  const uint16_t address = slot_address(slot);

  if (!backend_.read(address, &header, sizeof(header))) {
    return StorageStatus::IoError;
  }

  if (!header_has_valid_magic(header)) {
    return StorageStatus::InvalidMagic;
  }
  if (header.format_version != kStorageFormatVersion) {
    return StorageStatus::InvalidVersion;
  }
  if (header.slot_index != slot || header.payload_size != sizeof(PresetPayloadV1)) {
    return StorageStatus::InvalidPayloadSize;
  }

  if (!backend_.read(static_cast<uint16_t>(address + sizeof(header)), &payload, sizeof(payload))) {
    return StorageStatus::IoError;
  }

  const uint16_t crc = crc16_ccitt(reinterpret_cast<const uint8_t*>(&payload), sizeof(payload));
  if (crc != header.crc16) {
    return StorageStatus::InvalidCrc;
  }

  fill_project_from_payload(payload, project);
  return StorageStatus::Ok;
}

StorageStatus StorageEngine::validate_preset(uint8_t slot) const {
  if (slot >= kStoragePresetSlotCount) {
    return StorageStatus::InvalidSlot;
  }

  PresetSlotHeaderV1 header{};
  PresetPayloadV1 payload{};
  const uint16_t address = slot_address(slot);

  if (!backend_.read(address, &header, sizeof(header))) {
    return StorageStatus::IoError;
  }
  if (!header_has_valid_magic(header)) {
    return StorageStatus::InvalidMagic;
  }
  if (header.format_version != kStorageFormatVersion) {
    return StorageStatus::InvalidVersion;
  }
  if (header.slot_index != slot || header.payload_size != sizeof(PresetPayloadV1)) {
    return StorageStatus::InvalidPayloadSize;
  }
  if (!backend_.read(static_cast<uint16_t>(address + sizeof(header)), &payload, sizeof(payload))) {
    return StorageStatus::IoError;
  }

  const uint16_t crc = crc16_ccitt(reinterpret_cast<const uint8_t*>(&payload), sizeof(payload));
  return crc == header.crc16 ? StorageStatus::Ok : StorageStatus::InvalidCrc;
}

StorageStatus StorageEngine::save_metadata(const StorageMetadataV1& metadata) {
  if (!backend_.write(kStorageMetadataAddress, &metadata, sizeof(metadata))) {
    return StorageStatus::IoError;
  }
  return StorageStatus::Ok;
}

StorageStatus StorageEngine::load_metadata(StorageMetadataV1& metadata) const {
  if (!backend_.read(kStorageMetadataAddress, &metadata, sizeof(metadata))) {
    return StorageStatus::IoError;
  }
  if (!metadata_has_valid_magic(metadata)) {
    return StorageStatus::InvalidMagic;
  }
  if (metadata.storage_version != kStorageFormatVersion) {
    return StorageStatus::InvalidVersion;
  }
  return StorageStatus::Ok;
}

StorageMetadataV1 StorageEngine::build_default_metadata() {
  StorageMetadataV1 metadata{};
  memcpy(metadata.magic, kMetadataMagic, sizeof(metadata.magic));
  metadata.storage_version = kStorageFormatVersion;
  metadata.last_loaded_slot = 0xFFU;
  metadata.last_saved_slot = 0xFFU;
  metadata.flags = 0U;
  return metadata;
}

bool StorageEngine::preferred_startup_slot(const StorageMetadataV1& metadata, uint8_t& slot) {
  if (metadata.last_loaded_slot < kStoragePresetSlotCount) {
    slot = metadata.last_loaded_slot;
    return true;
  }

  if (metadata.last_saved_slot < kStoragePresetSlotCount) {
    slot = metadata.last_saved_slot;
    return true;
  }

  return false;
}

uint16_t StorageEngine::crc16_ccitt(const uint8_t* data, size_t size) {
  uint16_t crc = 0xFFFFU;
  for (size_t index = 0; index < size; ++index) {
    crc ^= static_cast<uint16_t>(data[index]) << 8U;
    for (uint8_t bit = 0; bit < 8U; ++bit) {
      crc = (crc & 0x8000U) != 0U ? static_cast<uint16_t>((crc << 1U) ^ 0x1021U)
                                  : static_cast<uint16_t>(crc << 1U);
    }
  }
  return crc;
}

uint16_t StorageEngine::slot_address(uint8_t slot) {
  return static_cast<uint16_t>(kStoragePresetBaseAddress + slot * kStoragePresetSlotSize);
}

void StorageEngine::fill_payload_from_project(const ProjectState& project, PresetPayloadV1& payload) {
  memset(&payload, 0, sizeof(payload));
  fill_track_storage_from_track(project.track_a, payload.track_a);
  fill_track_storage_from_track(project.track_b, payload.track_b);
  payload.machine_mode = static_cast<uint8_t>(project.machine_mode);
  payload.play_mode = static_cast<uint8_t>(project.play_mode);
  payload.tempo_bpm_x10 = project.tempo_bpm_x10;
  payload.root_note = project.root_note;
  payload.scale_id = project.scale_id;
}

void StorageEngine::fill_project_from_payload(const PresetPayloadV1& payload, ProjectState& project) {
  fill_track_from_storage(payload.track_a, project.track_a);
  fill_track_from_storage(payload.track_b, project.track_b);
  project.machine_mode = payload.machine_mode == static_cast<uint8_t>(MachineMode::Chain20)
                             ? MachineMode::Chain20
                             : MachineMode::Dual;
  switch (payload.play_mode) {
    case static_cast<uint8_t>(PlayMode::Reverse):
      project.play_mode = PlayMode::Reverse;
      break;
    case static_cast<uint8_t>(PlayMode::PingPong):
      project.play_mode = PlayMode::PingPong;
      break;
    case static_cast<uint8_t>(PlayMode::Random):
      project.play_mode = PlayMode::Random;
      break;
    case static_cast<uint8_t>(PlayMode::Forward):
    default:
      project.play_mode = PlayMode::Forward;
      break;
  }
  project.tempo_bpm_x10 = payload.tempo_bpm_x10 == 0U ? 1200U : payload.tempo_bpm_x10;
  project.root_note = payload.root_note;
  project.scale_id = payload.scale_id;
}

void StorageEngine::fill_track_storage_from_track(const Track& track, TrackStorageV1& storage_track) {
  memset(&storage_track, 0, sizeof(storage_track));
  for (size_t index = 0; index < kStepsPerTrack; ++index) {
    const Step& step = track.steps[index];
    StepStorageV1& storage_step = storage_track.steps[index];
    storage_step.flags = step.active ? kStepActiveFlag : 0U;
    storage_step.degree = step.degree;
    storage_step.probability = clamp_probability(step.probability);
    storage_step.ratchet = clamp_ratchet(step.ratchet);
    storage_step.gate = clamp_gate_percent(step.gate);
    storage_step.velocity = step.velocity;
  }

  storage_track.length = clamp_track_length(track.length);
  storage_track.midi_channel = clamp_midi_channel(track.midi_channel);
  storage_track.octave_offset = track.octave_offset;
}

void StorageEngine::fill_track_from_storage(const TrackStorageV1& storage_track, Track& track) {
  for (size_t index = 0; index < kStepsPerTrack; ++index) {
    const StepStorageV1& storage_step = storage_track.steps[index];
    Step& step = track.steps[index];
    step.active = (storage_step.flags & kStepActiveFlag) != 0U;
    step.degree = storage_step.degree;
    step.probability = clamp_probability(storage_step.probability);
    step.ratchet = clamp_ratchet(storage_step.ratchet);
    step.gate = clamp_gate_percent(storage_step.gate);
    step.velocity = storage_step.velocity;
  }

  track.length = clamp_track_length(storage_track.length);
  track.midi_channel = clamp_midi_channel(storage_track.midi_channel);
  track.octave_offset = storage_track.octave_offset;
}

bool StorageEngine::header_has_valid_magic(const PresetSlotHeaderV1& header) {
  return memcmp(header.magic, kPresetMagic, sizeof(header.magic)) == 0;
}

bool StorageEngine::metadata_has_valid_magic(const StorageMetadataV1& metadata) {
  return memcmp(metadata.magic, kMetadataMagic, sizeof(metadata.magic)) == 0;
}

}  // namespace dixpas
