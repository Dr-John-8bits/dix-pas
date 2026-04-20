#include "dixpas/app.hpp"
#include "dixpas/display_engine.hpp"
#include "dixpas/storage_engine.hpp"
#include "dixpas/ui_controller.hpp"

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

int main() {
  dixpas::App app;
  dixpas::MemoryStorageBackend storage_backend;
  dixpas::StorageEngine storage(storage_backend);
  dixpas::UiController ui(app, &storage);
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

  app.seed_demo_project();
  app.set_random_seed(0x1234ABCDU);
  storage.save_metadata(dixpas::StorageEngine::build_default_metadata());
  ui.reset();

  const dixpas::StorageStatus save_status = storage.save_preset(0, app.project());
  dixpas::ProjectState restored_project{};
  const dixpas::StorageStatus load_status = storage.load_preset(0, restored_project);

  std::printf("storage save=%u load=%u restoredTempo=%u\n", static_cast<unsigned>(save_status),
              static_cast<unsigned>(load_status), restored_project.tempo_bpm_x10);
  render_frame();

  ui.press_track_step(dixpas::TrackId::A, 0);
  render_frame();
  ui.rotate_encoder(2);
  render_frame();
  ui.press_mode_short();
  render_frame();
  ui.press_row3(0);
  render_frame();
  ui.update(1600);
  render_frame();

  ui.press_mode_long();
  render_frame();
  for (uint8_t index = 0; index < 6U; ++index) {
    ui.press_encoder_button();
  }
  render_frame();
  ui.rotate_encoder(1);
  render_frame();
  ui.set_shift_held(true);
  render_frame();
  ui.press_encoder_button();
  render_frame();
  ui.set_shift_held(false);
  render_frame();
  ui.press_mode_long();
  render_frame();

  ui.press_track_step(dixpas::TrackId::A, 0);
  ui.rotate_encoder(4);
  render_frame();
  ui.press_mode_long();
  render_frame();
  ui.press_encoder_button();
  render_frame();
  ui.press_mode_long();
  render_frame();

  app.set_clock_source(dixpas::ClockSource::Internal);
  ui.press_play();
  render_frame();
  drain_outputs("transport start");

  for (uint16_t i = 0; i < 8U * dixpas::kTicksPerStep; ++i) {
    if ((i % dixpas::kTicksPerStep) == 0U) {
      render_frame();
    }

    app.tick_internal();
    if ((i % 8U) == 0U) {
      drain_outputs("internal clock");
    }
  }

  ui.press_stop();
  render_frame();
  drain_outputs("transport stop");

  std::puts("external MIDI sync simulation");
  app.load_project(restored_project);
  ui.reset();
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
