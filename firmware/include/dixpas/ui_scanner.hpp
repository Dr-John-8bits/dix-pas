#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/fixed_queue.hpp"
#include "dixpas/types.hpp"
#include "dixpas/ui_controller.hpp"

namespace dixpas {

struct UiInputSnapshot {
  bool track_a_steps[kStepsPerTrack]{};
  bool track_b_steps[kStepsPerTrack]{};
  bool row3_steps[kStepsPerTrack]{};
  bool encoder_button = false;
  bool mode_button = false;
  bool shift_button = false;
  bool play_button = false;
  bool stop_button = false;
  bool reset_button = false;
  bool encoder_phase_a = false;
  bool encoder_phase_b = false;
};

enum class UiInputEventType : uint8_t {
  TrackStepPressed = 0,
  Row3Pressed = 1,
  EncoderTurned = 2,
  EncoderButtonPressed = 3,
  ModeShortPressed = 4,
  ModeLongPressed = 5,
  ShiftChanged = 6,
  PlayPressed = 7,
  StopPressed = 8,
  ResetPressed = 9,
};

struct UiInputEvent {
  UiInputEventType type = UiInputEventType::TrackStepPressed;
  TrackId track = TrackId::A;
  uint8_t index = 0;
  int8_t delta = 0;
  bool pressed = false;
};

class UiScanner {
 public:
  static constexpr uint16_t kDefaultDebounceMs = 20;
  static constexpr uint16_t kDefaultModeLongPressMs = 450;

  explicit UiScanner(uint16_t debounce_ms = kDefaultDebounceMs,
                     uint16_t mode_long_press_ms = kDefaultModeLongPressMs);

  void reset();
  void update(const UiInputSnapshot& snapshot, uint16_t elapsed_ms);
  bool pop_event(UiInputEvent& event) { return event_queue_.pop(event); }
  [[nodiscard]] bool has_overflowed() const { return event_queue_.overflowed(); }
  [[nodiscard]] uint32_t dropped_event_count() const { return event_queue_.dropped_count(); }
  void clear_overflow() { event_queue_.clear_overflow(); }

 private:
  static constexpr size_t kEventQueueCapacity = 32;

  struct ButtonState {
    bool raw = false;
    bool stable = false;
    uint16_t debounce_elapsed_ms = 0;
    uint16_t hold_elapsed_ms = 0;
    bool long_press_sent = false;
  };

  uint16_t debounce_ms_ = kDefaultDebounceMs;
  uint16_t mode_long_press_ms_ = kDefaultModeLongPressMs;
  ButtonState track_a_steps_[kStepsPerTrack]{};
  ButtonState track_b_steps_[kStepsPerTrack]{};
  ButtonState row3_steps_[kStepsPerTrack]{};
  ButtonState encoder_button_{};
  ButtonState mode_button_{};
  ButtonState shift_button_{};
  ButtonState play_button_{};
  ButtonState stop_button_{};
  ButtonState reset_button_{};
  uint8_t encoder_previous_state_ = 0;
  int8_t encoder_accumulator_ = 0;
  FixedQueue<UiInputEvent, kEventQueueCapacity> event_queue_{};

  void update_track_buttons(ButtonState* states, const bool* raw_steps, TrackId track,
                            uint16_t elapsed_ms);
  void update_row3_buttons(const bool* raw_steps, uint16_t elapsed_ms);
  void update_simple_button(ButtonState& state, bool raw_pressed, uint16_t elapsed_ms,
                            UiInputEventType event_type);
  void update_shift_button(bool raw_pressed, uint16_t elapsed_ms);
  void update_mode_button(bool raw_pressed, uint16_t elapsed_ms);
  void update_encoder(bool raw_phase_a, bool raw_phase_b);
  void push_event(const UiInputEvent& event);
  static void advance_button_timer(ButtonState& state, uint16_t elapsed_ms);
  bool commit_debounced_state(ButtonState& state, bool raw_pressed);
};

void dispatch_ui_event(UiController& ui, const UiInputEvent& event);

}  // namespace dixpas
