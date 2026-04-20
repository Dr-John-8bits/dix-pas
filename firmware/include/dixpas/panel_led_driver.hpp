#pragma once

#include <stdint.h>

#include "dixpas/app.hpp"
#include "dixpas/ui_controller.hpp"

namespace dixpas {

constexpr uint8_t kSystemLedCount = 4;

struct PanelLedFrame {
  bool track_a[kStepsPerTrack]{};
  bool track_b[kStepsPerTrack]{};
  bool system[kSystemLedCount]{};
};

class PanelLedDriver {
 public:
  void begin();
  void render(const App& app, const UiController& ui, bool blink_on, PanelLedFrame& frame) const;
  void write(const PanelLedFrame& frame) const;
};

}  // namespace dixpas
