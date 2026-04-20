#include "dixpas/startup_logic.hpp"

#include <stdio.h>
#include <string.h>

#include "dixpas/app.hpp"
#include "dixpas/storage_engine.hpp"

namespace dixpas {

namespace {

void set_message(StartupSelectionResult& result, const char* message) {
  snprintf(result.message, sizeof(result.message), "%s", message == nullptr ? "" : message);
}

uint8_t collect_startup_candidates(const StorageMetadataV1& metadata, uint8_t* slots,
                                   uint8_t capacity) {
  uint8_t count = 0U;

  if (count < capacity && metadata.last_loaded_slot < kStoragePresetSlotCount) {
    slots[count++] = metadata.last_loaded_slot;
  }

  if (count < capacity && metadata.last_saved_slot < kStoragePresetSlotCount &&
      metadata.last_saved_slot != metadata.last_loaded_slot) {
    slots[count++] = metadata.last_saved_slot;
  }

  return count;
}

}  // namespace

StartupSelectionResult apply_startup_project(App& app, StorageEngine* storage,
                                             bool storage_available,
                                             bool display_available) {
  StartupSelectionResult result{};
  result.storage_available = storage_available;
  result.display_available = display_available;

  app.load_default_project();
  result.fell_back_to_default = true;

  if (!storage_available || storage == nullptr) {
    set_message(result, display_available ? "No FRAM" : "No FRAM/OLED");
    return result;
  }

  StorageMetadataV1 metadata{};
  if (storage->load_metadata(metadata) != StorageStatus::Ok) {
    metadata = StorageEngine::build_default_metadata();
    storage->save_metadata(metadata);
  }

  uint8_t candidate_slots[2]{};
  const uint8_t candidate_count =
      collect_startup_candidates(metadata, candidate_slots, 2U);

  for (uint8_t index = 0U; index < candidate_count; ++index) {
    const uint8_t slot = candidate_slots[index];
    ProjectState startup_project{};
    if (storage->load_preset(slot, startup_project) != StorageStatus::Ok) {
      result.saw_preset_error = true;
      continue;
    }

    app.load_project(startup_project);
    metadata.last_loaded_slot = slot;
    storage->save_metadata(metadata);

    result.loaded_preset = true;
    result.fell_back_to_default = false;
    result.loaded_slot = slot;
    char value[32];
    snprintf(value, sizeof(value), "Loaded P%u", static_cast<unsigned>(slot + 1U));
    set_message(result, value);
    return result;
  }

  if (result.saw_preset_error) {
    set_message(result, "Preset Error");
  } else {
    set_message(result, "Default Project");
  }

  return result;
}

}  // namespace dixpas
