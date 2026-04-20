#include "dixpas/ui_scanner.hpp"

namespace dixpas {

namespace {

constexpr int8_t kQuadratureTable[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0,
};

}  // namespace

UiScanner::UiScanner(uint16_t debounce_ms, uint16_t mode_long_press_ms)
    : debounce_ms_(debounce_ms), mode_long_press_ms_(mode_long_press_ms) {
  reset();
}

void UiScanner::reset() {
  event_queue_.clear();

  for (ButtonState& state : track_a_steps_) {
    state = {};
  }
  for (ButtonState& state : track_b_steps_) {
    state = {};
  }
  for (ButtonState& state : row3_steps_) {
    state = {};
  }

  encoder_button_ = {};
  mode_button_ = {};
  shift_button_ = {};
  play_button_ = {};
  stop_button_ = {};
  reset_button_ = {};
  encoder_previous_state_ = 0U;
  encoder_accumulator_ = 0;
}

void UiScanner::update(const UiInputSnapshot& snapshot, uint16_t elapsed_ms) {
  update_track_buttons(track_a_steps_, snapshot.track_a_steps, TrackId::A, elapsed_ms);
  update_track_buttons(track_b_steps_, snapshot.track_b_steps, TrackId::B, elapsed_ms);
  update_row3_buttons(snapshot.row3_steps, elapsed_ms);
  update_simple_button(encoder_button_, snapshot.encoder_button, elapsed_ms,
                       UiInputEventType::EncoderButtonPressed);
  update_mode_button(snapshot.mode_button, elapsed_ms);
  update_shift_button(snapshot.shift_button, elapsed_ms);
  update_simple_button(play_button_, snapshot.play_button, elapsed_ms,
                       UiInputEventType::PlayPressed);
  update_simple_button(stop_button_, snapshot.stop_button, elapsed_ms,
                       UiInputEventType::StopPressed);
  update_simple_button(reset_button_, snapshot.reset_button, elapsed_ms,
                       UiInputEventType::ResetPressed);
  update_encoder(snapshot.encoder_phase_a, snapshot.encoder_phase_b);
}

void UiScanner::update_track_buttons(ButtonState* states, const bool* raw_steps, TrackId track,
                                     uint16_t elapsed_ms) {
  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    ButtonState& state = states[index];
    if (!commit_debounced_state(state, raw_steps[index])) {
      advance_button_timer(state, elapsed_ms);
      if (!commit_debounced_state(state, raw_steps[index])) {
        continue;
      }
    }

    if (state.stable) {
      UiInputEvent event{};
      event.type = UiInputEventType::TrackStepPressed;
      event.track = track;
      event.index = index;
      push_event(event);
    }
  }
}

void UiScanner::update_row3_buttons(const bool* raw_steps, uint16_t elapsed_ms) {
  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    ButtonState& state = row3_steps_[index];
    if (!commit_debounced_state(state, raw_steps[index])) {
      advance_button_timer(state, elapsed_ms);
      if (!commit_debounced_state(state, raw_steps[index])) {
        continue;
      }
    }

    if (state.stable) {
      UiInputEvent event{};
      event.type = UiInputEventType::Row3Pressed;
      event.index = index;
      push_event(event);
    }
  }
}

void UiScanner::update_simple_button(ButtonState& state, bool raw_pressed, uint16_t elapsed_ms,
                                     UiInputEventType event_type) {
  if (!commit_debounced_state(state, raw_pressed)) {
    advance_button_timer(state, elapsed_ms);
    if (!commit_debounced_state(state, raw_pressed)) {
      return;
    }
  }

  if (!state.stable) {
    return;
  }

  UiInputEvent event{};
  event.type = event_type;
  event.pressed = true;
  push_event(event);
}

void UiScanner::update_shift_button(bool raw_pressed, uint16_t elapsed_ms) {
  if (!commit_debounced_state(shift_button_, raw_pressed)) {
    advance_button_timer(shift_button_, elapsed_ms);
    if (!commit_debounced_state(shift_button_, raw_pressed)) {
      return;
    }
  }

  UiInputEvent event{};
  event.type = UiInputEventType::ShiftChanged;
  event.pressed = shift_button_.stable;
  push_event(event);
}

void UiScanner::update_mode_button(bool raw_pressed, uint16_t elapsed_ms) {
  if (mode_button_.raw != raw_pressed) {
    mode_button_.raw = raw_pressed;
    mode_button_.debounce_elapsed_ms = 0U;
  }

  advance_button_timer(mode_button_, elapsed_ms);

  if (mode_button_.raw != mode_button_.stable &&
      mode_button_.debounce_elapsed_ms >= debounce_ms_) {
    mode_button_.stable = mode_button_.raw;
    mode_button_.debounce_elapsed_ms = 0U;
    if (mode_button_.stable) {
      mode_button_.hold_elapsed_ms = 0U;
      mode_button_.long_press_sent = false;
    } else if (!mode_button_.long_press_sent) {
      UiInputEvent event{};
      event.type = UiInputEventType::ModeShortPressed;
      push_event(event);
    }
  }

  if (!mode_button_.stable || mode_button_.long_press_sent) {
    return;
  }

  const uint32_t new_hold_time =
      static_cast<uint32_t>(mode_button_.hold_elapsed_ms) + elapsed_ms;
  mode_button_.hold_elapsed_ms = new_hold_time > 0xFFFFU
                                     ? 0xFFFFU
                                     : static_cast<uint16_t>(new_hold_time);

  if (mode_button_.hold_elapsed_ms < mode_long_press_ms_) {
    return;
  }

  UiInputEvent event{};
  event.type = UiInputEventType::ModeLongPressed;
  push_event(event);
  mode_button_.long_press_sent = true;
}

void UiScanner::update_encoder(bool raw_phase_a, bool raw_phase_b) {
  const uint8_t current_state = static_cast<uint8_t>((raw_phase_a ? 0x02U : 0U) |
                                                     (raw_phase_b ? 0x01U : 0U));
  if (current_state == encoder_previous_state_) {
    return;
  }

  const uint8_t transition = static_cast<uint8_t>((encoder_previous_state_ << 2U) | current_state);
  encoder_previous_state_ = current_state;
  encoder_accumulator_ =
      static_cast<int8_t>(encoder_accumulator_ + kQuadratureTable[transition & 0x0FU]);

  while (encoder_accumulator_ >= 4) {
    UiInputEvent event{};
    event.type = UiInputEventType::EncoderTurned;
    event.delta = 1;
    push_event(event);
    encoder_accumulator_ = static_cast<int8_t>(encoder_accumulator_ - 4);
  }

  while (encoder_accumulator_ <= -4) {
    UiInputEvent event{};
    event.type = UiInputEventType::EncoderTurned;
    event.delta = -1;
    push_event(event);
    encoder_accumulator_ = static_cast<int8_t>(encoder_accumulator_ + 4);
  }
}

void UiScanner::push_event(const UiInputEvent& event) {
  event_queue_.push(event);
}

void UiScanner::advance_button_timer(ButtonState& state, uint16_t elapsed_ms) {
  const uint32_t new_time = static_cast<uint32_t>(state.debounce_elapsed_ms) + elapsed_ms;
  state.debounce_elapsed_ms =
      new_time > 0xFFFFU ? 0xFFFFU : static_cast<uint16_t>(new_time);
}

bool UiScanner::commit_debounced_state(ButtonState& state, bool raw_pressed) {
  if (state.raw != raw_pressed) {
    state.raw = raw_pressed;
    state.debounce_elapsed_ms = 0U;
    return false;
  }

  if (state.raw == state.stable || state.debounce_elapsed_ms < debounce_ms_) {
    return false;
  }

  state.stable = state.raw;
  state.debounce_elapsed_ms = 0U;
  state.hold_elapsed_ms = 0U;
  state.long_press_sent = false;
  return true;
}

void dispatch_ui_event(UiController& ui, const UiInputEvent& event) {
  switch (event.type) {
    case UiInputEventType::TrackStepPressed:
      ui.press_track_step(event.track, event.index);
      break;
    case UiInputEventType::Row3Pressed:
      ui.press_row3(event.index);
      break;
    case UiInputEventType::EncoderTurned:
      ui.rotate_encoder(event.delta);
      break;
    case UiInputEventType::EncoderButtonPressed:
      ui.press_encoder_button();
      break;
    case UiInputEventType::ModeShortPressed:
      ui.press_mode_short();
      break;
    case UiInputEventType::ModeLongPressed:
      ui.press_mode_long();
      break;
    case UiInputEventType::ShiftChanged:
      ui.set_shift_held(event.pressed);
      break;
    case UiInputEventType::PlayPressed:
      ui.press_play();
      break;
    case UiInputEventType::StopPressed:
      ui.press_stop();
      break;
    case UiInputEventType::ResetPressed:
      ui.press_reset();
      break;
  }
}

}  // namespace dixpas
