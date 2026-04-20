#pragma once

#include <stdint.h>

#include "dixpas/ui_scanner.hpp"

namespace dixpas {

class UiHardware {
 public:
  void begin();
  [[nodiscard]] UiInputSnapshot read_snapshot() const;
};

}  // namespace dixpas
