#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/app.hpp"
#include "dixpas/ui_controller.hpp"

namespace dixpas {

constexpr size_t kDisplayLineCount = 4;
constexpr size_t kDisplayLineWidth = 21;

struct DisplayFrame {
  char lines[kDisplayLineCount][kDisplayLineWidth + 1]{};
};

class DisplayEngine {
 public:
  void render(const App& app, const UiController& ui, DisplayFrame& frame) const;
  static bool equals(const DisplayFrame& lhs, const DisplayFrame& rhs);
};

}  // namespace dixpas
