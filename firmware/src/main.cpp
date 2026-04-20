#include "dixpas/app.hpp"

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdio>
#endif

namespace {

#if defined(ARDUINO)
dixpas::App g_app;
uint32_t g_last_tick_micros = 0;

void drain_engine_events() {
  dixpas::EngineEvent event;
  while (g_app.pop_event(event)) {
    // TODO: Route note and gate events to MIDI DIN, gate outputs, and LEDs.
    (void)event;
  }
}
#endif

}  // namespace

#if defined(ARDUINO)

void setup() {
  g_app.seed_demo_project();
  g_app.start();
  g_last_tick_micros = micros();
}

void loop() {
  const uint32_t tick_interval = g_app.clock().internal_tick_interval_micros();
  const uint32_t now = micros();

  while (static_cast<uint32_t>(now - g_last_tick_micros) >= tick_interval) {
    g_last_tick_micros += tick_interval;
    g_app.tick();
  }

  drain_engine_events();
}

#else

int main() {
  dixpas::App app;
  app.seed_demo_project();
  app.set_random_seed(0x1234ABCDU);
  app.start();

  std::puts("DIX PAS native simulation");

  for (uint16_t i = 0; i < 8U * dixpas::kTicksPerStep; ++i) {
    dixpas::EngineEvent event;
    while (app.pop_event(event)) {
      const char* type = "";
      switch (event.type) {
        case dixpas::EventType::NoteOn:
          type = "NOTE_ON ";
          break;
        case dixpas::EventType::NoteOff:
          type = "NOTE_OFF";
          break;
        case dixpas::EventType::GateHigh:
          type = "GATE_ON ";
          break;
        case dixpas::EventType::GateLow:
          type = "GATE_OFF";
          break;
      }

      const char track_name = event.track == dixpas::TrackId::A ? 'A' : 'B';
      std::printf("tick=%3lu track=%c type=%s ch=%u note=%u vel=%u\n",
                  static_cast<unsigned long>(event.tick), track_name, type, event.midi_channel,
                  event.note, event.velocity);
    }

    app.tick();
  }

  app.stop();
  return 0;
}

#endif
