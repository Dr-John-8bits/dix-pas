#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/types.hpp"

namespace dixpas {

constexpr uint8_t kStorageFormatVersion = 1;
constexpr uint8_t kStoragePresetSlotCount = 8;
constexpr uint16_t kStorageMetadataAddress = 0x0000;
constexpr uint16_t kStoragePresetBaseAddress = 0x0100;
constexpr uint16_t kStoragePresetSlotSize = 0x0100;
constexpr size_t kMemoryStorageCapacity =
    kStoragePresetBaseAddress + kStoragePresetSlotSize * kStoragePresetSlotCount;

enum class StorageStatus : uint8_t {
  Ok = 0,
  InvalidSlot,
  IoError,
  InvalidMagic,
  InvalidVersion,
  InvalidPayloadSize,
  InvalidCrc,
};

struct StepStorageV1 {
  uint8_t flags;
  uint8_t degree;
  uint8_t probability;
  uint8_t ratchet;
  uint8_t gate;
  uint8_t velocity;
} __attribute__((packed));

struct TrackStorageV1 {
  StepStorageV1 steps[kStepsPerTrack];
  uint8_t length;
  uint8_t midi_channel;
  int8_t octave_offset;
  uint8_t reserved;
} __attribute__((packed));

struct PresetPayloadV1 {
  TrackStorageV1 track_a;
  TrackStorageV1 track_b;
  uint8_t machine_mode;
  uint8_t play_mode;
  uint16_t tempo_bpm_x10;
  uint8_t root_note;
  uint8_t scale_id;
  uint8_t reserved[8];
} __attribute__((packed));

struct PresetSlotHeaderV1 {
  char magic[4];
  uint8_t format_version;
  uint8_t slot_index;
  uint16_t payload_size;
  uint16_t crc16;
  uint8_t flags;
  uint8_t reserved[5];
} __attribute__((packed));

struct StorageMetadataV1 {
  char magic[4];
  uint8_t storage_version;
  uint8_t last_loaded_slot;
  uint8_t last_saved_slot;
  uint8_t flags;
  uint8_t reserved[8];
} __attribute__((packed));

class StorageBackend {
 public:
  virtual ~StorageBackend() = default;
  virtual bool read(uint16_t address, void* dst, size_t size) = 0;
  virtual bool write(uint16_t address, const void* src, size_t size) = 0;
};

class MemoryStorageBackend : public StorageBackend {
 public:
  MemoryStorageBackend();

  bool read(uint16_t address, void* dst, size_t size) override;
  bool write(uint16_t address, const void* src, size_t size) override;
  void clear(uint8_t fill = 0xFF);

 private:
  uint8_t bytes_[kMemoryStorageCapacity]{};
};

class StorageEngine {
 public:
  explicit StorageEngine(StorageBackend& backend) : backend_(backend) {}

  StorageStatus save_preset(uint8_t slot, const ProjectState& project);
  StorageStatus load_preset(uint8_t slot, ProjectState& project);
  StorageStatus validate_preset(uint8_t slot) const;

  StorageStatus save_metadata(const StorageMetadataV1& metadata);
  StorageStatus load_metadata(StorageMetadataV1& metadata) const;
  static StorageMetadataV1 build_default_metadata();

  static uint16_t crc16_ccitt(const uint8_t* data, size_t size);

 private:
  StorageBackend& backend_;

  static uint16_t slot_address(uint8_t slot);
  static void fill_payload_from_project(const ProjectState& project, PresetPayloadV1& payload);
  static void fill_project_from_payload(const PresetPayloadV1& payload, ProjectState& project);
  static void fill_track_storage_from_track(const Track& track, TrackStorageV1& storage_track);
  static void fill_track_from_storage(const TrackStorageV1& storage_track, Track& track);
  static bool header_has_valid_magic(const PresetSlotHeaderV1& header);
  static bool metadata_has_valid_magic(const StorageMetadataV1& metadata);
};

}  // namespace dixpas
