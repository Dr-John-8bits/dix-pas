#pragma once

#include <stdint.h>

namespace dixpas {

class App;
class StorageEngine;

struct StartupSelectionResult {
  bool storage_available = false;
  bool display_available = false;
  bool loaded_preset = false;
  bool fell_back_to_default = false;
  bool saw_preset_error = false;
  uint8_t loaded_slot = 0xFFU;
  char message[32]{};
};

StartupSelectionResult apply_startup_project(App& app, StorageEngine* storage,
                                             bool storage_available,
                                             bool display_available);

}  // namespace dixpas
