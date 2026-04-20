#include "dixpas/app.hpp"
#include "dixpas/storage_engine.hpp"

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

  app.seed_demo_project();
  app.set_random_seed(0x1234ABCDU);

  const dixpas::StorageStatus save_status = storage.save_preset(0, app.project());
  dixpas::ProjectState restored_project{};
  const dixpas::StorageStatus load_status = storage.load_preset(0, restored_project);

  std::printf("storage save=%u load=%u restoredTempo=%u\n", static_cast<unsigned>(save_status),
              static_cast<unsigned>(load_status), restored_project.tempo_bpm_x10);

  app.set_clock_source(dixpas::ClockSource::Internal);
  app.start();

  std::puts("DIX PAS native simulation");

  for (uint16_t i = 0; i < 8U * dixpas::kTicksPerStep; ++i) {
    dixpas::EngineEvent event;
    while (app.pop_routed_event(event)) {
      const char track_name = event.track == dixpas::TrackId::A ? 'A' : 'B';
      std::printf("tick=%3lu track=%c type=%s ch=%u note=%u vel=%u\n",
                  static_cast<unsigned long>(event.tick), track_name, event_type_name(event.type),
                  event.midi_channel,
                  event.note, event.velocity);
    }

    uint8_t midi_byte = 0;
    while (app.pop_midi_byte(midi_byte)) {
      std::printf("midi=0x%02X\n", midi_byte);
    }

    app.tick_internal();
  }

  app.stop();

  std::puts("external MIDI sync simulation");
  app.load_project(restored_project);
  app.set_clock_source(dixpas::ClockSource::ExternalMidi);
  app.receive_midi_byte(0xFAU);
  for (uint8_t clock = 0; clock < 8U; ++clock) {
    app.receive_midi_byte(0xF8U);
  }
  app.receive_midi_byte(0xFCU);
  return 0;
}

#endif
