#include "dixpas/app.hpp"
#include "dixpas/display_engine.hpp"
#include "dixpas/fram_i2c_backend.hpp"
#include "dixpas/generative_engine.hpp"
#include "dixpas/music_scales.hpp"
#include "dixpas/oled_display.hpp"
#include "dixpas/panel_led_driver.hpp"
#include "dixpas/storage_engine.hpp"
#include "dixpas/ui_hardware.hpp"
#include "dixpas/ui_controller.hpp"
#include "dixpas/ui_scanner.hpp"

#if defined(ARDUINO)
#include <Arduino.h>
#elif !defined(PIO_UNIT_TESTING)
#include <cstdio>
#endif

namespace {

#if defined(ARDUINO)
constexpr uint8_t kGateOutAPin = 5;
constexpr uint8_t kGateOutBPin = 6;
constexpr uint16_t kBootSplashDurationMs = 1500U;

dixpas::App g_app;
dixpas::WireFramI2cPort g_fram_port;
dixpas::FramI2cBackend g_fram_backend(g_fram_port);
dixpas::StorageEngine g_storage(g_fram_backend);
dixpas::UiController g_ui(g_app);
dixpas::UiScanner g_ui_scanner;
dixpas::UiHardware g_ui_hardware;
dixpas::WireOledI2cPort g_oled_port;
dixpas::OledDisplay g_oled_display(g_oled_port);
dixpas::DisplayEngine g_display_engine;
dixpas::DisplayFrame g_display_frame{};
dixpas::DisplayFrame g_previous_display_frame{};
dixpas::PanelLedDriver g_panel_led_driver;
dixpas::PanelLedFrame g_panel_led_frame{};
uint32_t g_last_tick_micros = 0;
uint32_t g_last_ui_update_millis = 0;
uint32_t g_boot_deadline_millis = 0;
bool g_boot_active = true;

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

void sync_panel_leds() {
  const bool blink_on = ((millis() / 120U) % 2U) == 0U;
  g_panel_led_driver.render(g_app, g_ui, blink_on, g_panel_led_frame);
  g_panel_led_driver.write(g_panel_led_frame);
}

void sync_display() {
  g_display_engine.render(g_app, g_ui, g_display_frame);
  if (dixpas::DisplayEngine::equals(g_display_frame, g_previous_display_frame)) {
    return;
  }

  if (g_oled_display.is_ready()) {
    g_oled_display.render(g_display_frame);
  }
  g_previous_display_frame = g_display_frame;
}

void process_ui_inputs() {
  const uint32_t now = millis();
  const uint32_t elapsed = now - g_last_ui_update_millis;
  if (elapsed == 0U) {
    return;
  }

  g_last_ui_update_millis = now;
  g_ui.update(static_cast<uint16_t>(elapsed > 0xFFFFU ? 0xFFFFU : elapsed));

  const dixpas::UiInputSnapshot snapshot = g_ui_hardware.read_snapshot();
  g_ui_scanner.update(snapshot, static_cast<uint16_t>(elapsed > 0xFFFFU ? 0xFFFFU : elapsed));

  dixpas::UiInputEvent event;
  while (g_ui_scanner.pop_event(event)) {
    dixpas::dispatch_ui_event(g_ui, event);
  }
}

void initialize_storage_and_startup_project() {
  if (!g_fram_backend.begin()) {
    return;
  }

  dixpas::StorageMetadataV1 metadata{};
  if (g_storage.load_metadata(metadata) != dixpas::StorageStatus::Ok) {
    metadata = dixpas::StorageEngine::build_default_metadata();
    g_storage.save_metadata(metadata);
  }

  uint8_t startup_slot = 0U;
  if (dixpas::StorageEngine::preferred_startup_slot(metadata, startup_slot)) {
    dixpas::ProjectState startup_project{};
    if (g_storage.load_preset(startup_slot, startup_project) == dixpas::StorageStatus::Ok) {
      g_app.load_project(startup_project);
      metadata.last_loaded_slot = startup_slot;
      g_storage.save_metadata(metadata);
    }
  }

  g_ui.attach_storage(g_storage);
}
#endif

}  // namespace

#if defined(ARDUINO)

void setup() {
  pinMode(kGateOutAPin, OUTPUT);
  pinMode(kGateOutBPin, OUTPUT);
  digitalWrite(kGateOutAPin, LOW);
  digitalWrite(kGateOutBPin, LOW);

  g_app.seed_demo_project();
  Serial1.begin(31250);
  g_oled_display.begin();
  g_ui_hardware.begin();
  g_panel_led_driver.begin();
  g_ui.reset();
  initialize_storage_and_startup_project();
  g_ui.reset();
  g_ui.enter_boot_page();
  g_ui_scanner.reset();
  g_last_tick_micros = micros();
  g_last_ui_update_millis = millis();
  g_boot_deadline_millis = g_last_ui_update_millis + kBootSplashDurationMs;
  g_boot_active = true;
  sync_gate_outputs();
  sync_display();
  sync_panel_leds();
}

void loop() {
  while (Serial1.available() > 0) {
    const int byte_read = Serial1.read();
    if (byte_read >= 0) {
      g_app.receive_midi_byte(static_cast<uint8_t>(byte_read));
    }
  }

  if (g_boot_active) {
    const uint32_t now = millis();
    if (static_cast<int32_t>(now - g_boot_deadline_millis) < 0) {
      flush_midi_bytes();
      sync_gate_outputs();
      sync_display();
      sync_panel_leds();
      return;
    }

    g_boot_active = false;
    g_ui.leave_boot_page();
    g_last_ui_update_millis = now;
  }

  process_ui_inputs();

  if (g_app.clock_source() == dixpas::ClockSource::Internal) {
    if (g_app.transport_state() != dixpas::TransportState::Playing) {
      g_last_tick_micros = micros();
    } else {
      const uint32_t tick_interval = g_app.clock().internal_tick_interval_micros();
      const uint32_t now = micros();

      while (static_cast<uint32_t>(now - g_last_tick_micros) >= tick_interval) {
        g_last_tick_micros += tick_interval;
        g_app.tick_internal();
      }
    }
  } else {
    g_last_tick_micros = micros();
  }

  flush_midi_bytes();
  sync_gate_outputs();
  sync_display();
  sync_panel_leds();
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

const char* contour_name(dixpas::MelodyContour contour) {
  switch (contour) {
    case dixpas::MelodyContour::Ascending:
      return "Ascending";
    case dixpas::MelodyContour::Descending:
      return "Descending";
    case dixpas::MelodyContour::PingPong:
      return "PingPong";
    case dixpas::MelodyContour::Random:
      return "Random";
    case dixpas::MelodyContour::RandomWalk:
      return "RandomWalk";
    case dixpas::MelodyContour::Alternating:
      return "Alternating";
  }

  return "Contour";
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

  auto cycle_global_target_to = [&](dixpas::GlobalTarget target, const char* label) {
    uint8_t attempts = 0U;
    while (ui.global_target() != target && attempts < 16U) {
      tap_button(input.mode_button, label);
      ++attempts;
    }

    if (ui.global_target() != target) {
      std::printf("warning: unable to reach global target %u\n",
                  static_cast<unsigned>(target));
    }
  };

  auto print_generated_track = [&](const char* label, const dixpas::ProjectState& project,
                                   dixpas::TrackId track_id) {
    const dixpas::Track& track =
        track_id == dixpas::TrackId::A ? project.track_a : project.track_b;
    const char track_name = track_id == dixpas::TrackId::A ? 'A' : 'B';
    std::printf("== %s / Track %c ==\n", label, track_name);
    for (uint8_t index = 0; index < track.length; ++index) {
      const dixpas::Step& step = track.steps[index];
      const uint8_t midi_note = dixpas::resolve_scale_note(project.root_note, project.scale_id,
                                                           step.degree, track.octave_offset);
      const int octave = static_cast<int>(midi_note / 12U) - 1;
      std::printf("step=%02u active=%u degree=%02u note=%s%d prob=%u gate=%u vel=%u\n",
                  static_cast<unsigned>(index + 1U), step.active ? 1U : 0U,
                  static_cast<unsigned>(step.degree), dixpas::note_name(midi_note), octave,
                  static_cast<unsigned>(step.probability), static_cast<unsigned>(step.gate),
                  static_cast<unsigned>(step.velocity));
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

  std::puts("generative engine simulation");
  long_press_button(input.mode_button, 500U, "mode long");
  cycle_global_target_to(dixpas::GlobalTarget::GenerativeSlot, "mode short");
  tap_button(input.encoder_button, "apply generative");
  print_generated_track("generated", app.project(), dixpas::TrackId::A);
  print_generated_track("generated", app.project(), dixpas::TrackId::B);

  input.shift_button = true;
  advance_ui(25U, "shift mutate");
  tap_button(input.encoder_button, "mutate generative");
  input.shift_button = false;
  advance_ui(25U, "shift release");
  print_generated_track("mutated", app.project(), dixpas::TrackId::A);

  rotate_encoder_steps(1, "generative slot");
  tap_button(input.encoder_button, "apply generative");
  print_generated_track("generated", app.project(), dixpas::TrackId::A);
  print_generated_track("generated", app.project(), dixpas::TrackId::B);
  return 0;
}

#endif
