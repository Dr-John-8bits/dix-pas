#include "dixpas/app.hpp"
#include "dixpas/display_engine.hpp"
#include "dixpas/storage_engine.hpp"
#include "dixpas/ui_controller.hpp"
#include "dixpas/ui_scanner.hpp"

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdio>
#endif

namespace {

#if defined(ARDUINO)
constexpr uint8_t kGateOutAPin = 5;
constexpr uint8_t kGateOutBPin = 6;

dixpas::App g_app;
uint32_t g_last_tick_micros = 0;

void flush_midi_bytes() {
  uint8_t byte = 0;
  while (g_app.pop_midi_byte(byte)) {
    Serial1.write(byte);
  }
}

void sync_gate_outputs() {
  digitalWrite(kGateOutAPin, g_app.gate_state(dixpas::TrackId::A) ? HIGH : LOW);
  digitalWrite(kGateOutBPin, g_app.gate_state(dixpas::TrackId::B) ? HIGH : LOW);
}
#endif

}  // namespace

#if defined(ARDUINO)

void setup() {
  pinMode(kGateOutAPin, OUTPUT);
  pinMode(kGateOutBPin, OUTPUT);
  digitalWrite(kGateOutAPin, LOW);
  digitalWrite(kGateOutBPin, LOW);

  Serial1.begin(31250);

  g_app.seed_demo_project();
  g_app.start();
  g_last_tick_micros = micros();
  sync_gate_outputs();
}

void loop() {
  while (Serial1.available() > 0) {
    const int byte_read = Serial1.read();
    if (byte_read >= 0) {
      g_app.receive_midi_byte(static_cast<uint8_t>(byte_read));
    }
  }

  if (g_app.clock_source() == dixpas::ClockSource::Internal) {
    const uint32_t tick_interval = g_app.clock().internal_tick_interval_micros();
    const uint32_t now = micros();

    while (static_cast<uint32_t>(now - g_last_tick_micros) >= tick_interval) {
      g_last_tick_micros += tick_interval;
      g_app.tick_internal();
    }
  }

  flush_midi_bytes();
  sync_gate_outputs();
}

#else

const char* event_type_name(dixpas::EventType type) {
  switch (type) {
    case dixpas::EventType::NoteOn:
      return "NOTE_ON ";
    case dixpas::EventType::NoteOff:
      return "NOTE_OFF";
    case dixpas::EventType::GateHigh:
      return "GATE_ON ";
    case dixpas::EventType::GateLow:
      return "GATE_OFF";
  }

  return "UNKNOWN ";
}

const char* ui_event_name(dixpas::UiInputEventType type) {
  switch (type) {
    case dixpas::UiInputEventType::TrackStepPressed:
      return "TRACK_STEP";
    case dixpas::UiInputEventType::Row3Pressed:
      return "ROW3";
    case dixpas::UiInputEventType::EncoderTurned:
      return "ENCODER";
    case dixpas::UiInputEventType::EncoderButtonPressed:
      return "ENC_BTN";
    case dixpas::UiInputEventType::ModeShortPressed:
      return "MODE_SHORT";
    case dixpas::UiInputEventType::ModeLongPressed:
      return "MODE_LONG";
    case dixpas::UiInputEventType::ShiftChanged:
      return "SHIFT";
    case dixpas::UiInputEventType::PlayPressed:
      return "PLAY";
    case dixpas::UiInputEventType::StopPressed:
      return "STOP";
    case dixpas::UiInputEventType::ResetPressed:
      return "RESET";
  }

  return "UI";
}

int main() {
  dixpas::App app;
  dixpas::MemoryStorageBackend storage_backend;
  dixpas::StorageEngine storage(storage_backend);
  dixpas::UiController ui(app, &storage);
  dixpas::UiScanner scanner;
  dixpas::UiInputSnapshot input{};
  dixpas::DisplayEngine display;
  dixpas::DisplayFrame frame{};
  dixpas::DisplayFrame previous_frame{};

  auto print_frame = [](const dixpas::DisplayFrame& current_frame) {
    std::puts("----- OLED -----");
    for (size_t line = 0; line < dixpas::kDisplayLineCount; ++line) {
      std::printf("%s\n", current_frame.lines[line]);
    }
  };

  auto render_frame = [&]() {
    display.render(app, ui, frame);
    if (!dixpas::DisplayEngine::equals(frame, previous_frame)) {
      print_frame(frame);
      previous_frame = frame;
    }
  };

  auto drain_outputs = [&](const char* label) {
    bool printed_label = false;
    dixpas::EngineEvent event;
    while (app.pop_routed_event(event)) {
      if (!printed_label) {
        std::printf("== %s ==\n", label);
        printed_label = true;
      }

      const char track_name = event.track == dixpas::TrackId::A ? 'A' : 'B';
      std::printf("tick=%3lu track=%c type=%s ch=%u note=%u vel=%u\n",
                  static_cast<unsigned long>(event.tick), track_name, event_type_name(event.type),
                  event.midi_channel, event.note, event.velocity);
    }

    uint8_t midi_byte = 0;
    while (app.pop_midi_byte(midi_byte)) {
      if (!printed_label) {
        std::printf("== %s ==\n", label);
        printed_label = true;
      }
      std::printf("midi=0x%02X\n", midi_byte);
    }
  };

  auto flush_scanner_events = [&](const char* label) {
    bool printed_label = false;
    dixpas::UiInputEvent event;
    while (scanner.pop_event(event)) {
      if (!printed_label) {
        std::printf("== %s ==\n", label);
        printed_label = true;
      }

      std::printf("ui=%s", ui_event_name(event.type));
      if (event.type == dixpas::UiInputEventType::TrackStepPressed) {
        std::printf(" track=%c index=%u", event.track == dixpas::TrackId::A ? 'A' : 'B',
                    static_cast<unsigned>(event.index + 1U));
      } else if (event.type == dixpas::UiInputEventType::Row3Pressed) {
        std::printf(" index=%u", static_cast<unsigned>(event.index + 1U));
      } else if (event.type == dixpas::UiInputEventType::EncoderTurned) {
        std::printf(" delta=%d", static_cast<int>(event.delta));
      } else if (event.type == dixpas::UiInputEventType::ShiftChanged) {
        std::printf(" pressed=%u", event.pressed ? 1U : 0U);
      }
      std::printf("\n");

      dixpas::dispatch_ui_event(ui, event);
    }

    render_frame();
    drain_outputs(label);
  };

  auto advance_ui = [&](uint16_t elapsed_ms, const char* label) {
    ui.update(elapsed_ms);
    scanner.update(input, elapsed_ms);
    flush_scanner_events(label);
  };

  auto tap_button = [&](bool& button, const char* label) {
    button = true;
    advance_ui(25U, label);
    button = false;
    advance_ui(25U, label);
  };

  auto long_press_button = [&](bool& button, uint16_t hold_ms, const char* label) {
    button = true;
    advance_ui(25U, label);
    advance_ui(hold_ms, label);
    button = false;
    advance_ui(25U, label);
  };

  auto rotate_encoder_steps = [&](int8_t steps, const char* label) {
    static constexpr uint8_t kClockwiseSequence[5][2] = {
        {0U, 0U}, {1U, 0U}, {1U, 1U}, {0U, 1U}, {0U, 0U},
    };
    static constexpr uint8_t kCounterClockwiseSequence[5][2] = {
        {0U, 0U}, {0U, 1U}, {1U, 1U}, {1U, 0U}, {0U, 0U},
    };

    const uint8_t (*sequence)[2] =
        steps >= 0 ? kClockwiseSequence : kCounterClockwiseSequence;
    const uint8_t step_count = steps >= 0 ? static_cast<uint8_t>(steps)
                                          : static_cast<uint8_t>(-steps);

    for (uint8_t step = 0; step < step_count; ++step) {
      for (uint8_t state_index = 0; state_index < 5U; ++state_index) {
        input.encoder_phase_a = sequence[state_index][0] != 0U;
        input.encoder_phase_b = sequence[state_index][1] != 0U;
        advance_ui(1U, label);
      }
    }
  };

  app.seed_demo_project();
  app.set_random_seed(0x1234ABCDU);
  storage.save_metadata(dixpas::StorageEngine::build_default_metadata());
  ui.reset();
  scanner.reset();

  const dixpas::StorageStatus save_status = storage.save_preset(0, app.project());
  dixpas::ProjectState restored_project{};
  const dixpas::StorageStatus load_status = storage.load_preset(0, restored_project);

  std::printf("storage save=%u load=%u restoredTempo=%u\n", static_cast<unsigned>(save_status),
              static_cast<unsigned>(load_status), restored_project.tempo_bpm_x10);
  render_frame();

  tap_button(input.track_a_steps[0], "track step select");
  rotate_encoder_steps(2, "encoder rotate");
  tap_button(input.mode_button, "mode short");
  tap_button(input.row3_steps[0], "row3 toggle");
  advance_ui(1600U, "ui timeout");

  long_press_button(input.mode_button, 500U, "mode long");
  for (uint8_t index = 0; index < 6U; ++index) {
    tap_button(input.encoder_button, "encoder button");
  }
  rotate_encoder_steps(1, "preset slot");
  input.shift_button = true;
  advance_ui(25U, "shift save");
  tap_button(input.encoder_button, "preset save");
  input.shift_button = false;
  advance_ui(25U, "shift release");
  long_press_button(input.mode_button, 500U, "mode long");

  tap_button(input.track_a_steps[0], "track step select");
  rotate_encoder_steps(4, "encoder rotate");
  long_press_button(input.mode_button, 500U, "mode long");
  tap_button(input.encoder_button, "preset load");
  long_press_button(input.mode_button, 500U, "mode long");

  app.set_clock_source(dixpas::ClockSource::Internal);
  tap_button(input.play_button, "play button");

  for (uint16_t i = 0; i < 8U * dixpas::kTicksPerStep; ++i) {
    if ((i % dixpas::kTicksPerStep) == 0U) {
      render_frame();
    }

    app.tick_internal();
    if ((i % 8U) == 0U) {
      drain_outputs("internal clock");
    }
  }

  tap_button(input.stop_button, "stop button");

  std::puts("external MIDI sync simulation");
  app.load_project(restored_project);
  ui.reset();
  scanner.reset();
  input = {};
  app.set_clock_source(dixpas::ClockSource::ExternalMidi);
  render_frame();
  app.receive_midi_byte(0xFAU);
  render_frame();
  for (uint8_t clock = 0; clock < 8U; ++clock) {
    app.receive_midi_byte(0xF8U);
    drain_outputs("external clock");
  }
  app.receive_midi_byte(0xFCU);
  render_frame();
  drain_outputs("external stop");
  return 0;
}

#endif
