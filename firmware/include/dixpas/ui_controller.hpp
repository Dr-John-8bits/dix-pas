#pragma once

#include <stdint.h>

#include "dixpas/app.hpp"
#include "dixpas/storage_engine.hpp"
#include "dixpas/types.hpp"

namespace dixpas {

enum class Row3BaseMode : uint8_t {
  Probability = 0,
  OnOff = 1,
};

enum class EncoderTarget : uint8_t {
  Note = 0,
  Gate = 1,
  Velocity = 2,
};

enum class GlobalTarget : uint8_t {
  Tempo = 0,
  Root = 1,
  Scale = 2,
  PlayMode = 3,
  TrackLength = 4,
  MidiChannel = 5,
  PresetSlot = 6,
  GenerativeSlot = 7,
};

enum class UiPage : uint8_t {
  Boot = 0,
  Home = 1,
  StepEdit = 2,
  GlobalEdit = 3,
};

struct UiOverlay {
  bool active = false;
  char line1[32]{};
  char line2[32]{};
};

class UiController {
 public:
  explicit UiController(App& app, StorageEngine* storage = nullptr);

  void reset();
  void update(uint16_t elapsed_ms);
  void attach_storage(StorageEngine& storage);

  void press_track_step(TrackId track, uint8_t index);
  void press_row3(uint8_t index);
  void rotate_encoder(int8_t delta);
  void press_encoder_button();
  void press_mode_short();
  void press_mode_long();
  void set_shift_held(bool held);
  void press_play();
  void press_stop();
  void press_reset();

  [[nodiscard]] UiPage page() const { return page_; }
  [[nodiscard]] TrackId track_focus() const { return track_focus_; }
  [[nodiscard]] uint8_t selected_step(TrackId track) const;
  [[nodiscard]] Row3BaseMode row3_base_mode() const { return row3_base_mode_; }
  [[nodiscard]] EncoderTarget encoder_target() const { return encoder_target_; }
  [[nodiscard]] GlobalTarget global_target() const { return global_target_; }
  [[nodiscard]] bool shift_held() const { return shift_held_; }
  [[nodiscard]] uint8_t preset_slot() const { return preset_slot_; }
  [[nodiscard]] uint8_t generative_slot() const { return generative_slot_; }
  [[nodiscard]] const UiOverlay& overlay() const { return overlay_; }

 private:
  static constexpr uint16_t kOverlayTimeoutMs = 1500;

  App& app_;
  ProjectState project_{};
  UiPage page_ = UiPage::Home;
  TrackId track_focus_ = TrackId::A;
  uint8_t selected_step_a_ = 0;
  uint8_t selected_step_b_ = 0;
  Row3BaseMode row3_base_mode_ = Row3BaseMode::Probability;
  EncoderTarget encoder_target_ = EncoderTarget::Note;
  GlobalTarget global_target_ = GlobalTarget::Tempo;
  bool shift_held_ = false;
  uint8_t preset_slot_ = 0;
  uint8_t generative_slot_ = 0;
  UiOverlay overlay_{};
  uint16_t overlay_timeout_ms_ = 0;
  StorageEngine* storage_ = nullptr;

  void commit_project(bool reset_playhead);
  void show_step_overlay(TrackId track, uint8_t step_index, const char* parameter,
                         const char* value);
  void show_global_overlay(const char* parameter, const char* value);
  void show_transport_overlay(const char* label);
  void load_selected_preset();
  void save_selected_preset();
  void apply_selected_generative_slot();
  void mutate_selected_generative_slot();
  void update_storage_metadata(bool saved);
  void show_storage_overlay(const char* action, StorageStatus status);
  void restore_preset_slot_from_metadata();

  Track& focused_track();
  const Track& focused_track() const;
  uint8_t& focused_selected_step();
  uint8_t focused_selected_step() const;
  Step& selected_step_ref();

  void cycle_probability(Step& step);
  void cycle_ratchet(Step& step);
  void edit_step_value(int8_t delta);
  void edit_global_value(int8_t delta);
};

}  // namespace dixpas
