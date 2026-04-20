#include "dixpas/app.hpp"

namespace dixpas {

App::App() {
  load_project(build_default_project());
}

void App::load_project(const ProjectState& project) {
  clock_.set_tempo_bpm_x10(project.tempo_bpm_x10);
  sequencer_.load_project(project);
}

void App::set_clock_source(ClockSource source) {
  clock_.set_clock_source(source);
}

void App::seed_demo_project() {
  ProjectState project = build_default_project();

  project.track_a.length = 8;
  project.track_b.length = 5;
  project.track_a.midi_channel = 1;
  project.track_b.midi_channel = 2;
  project.root_note = 0;
  project.scale_id = 0;
  project.play_mode = PlayMode::Forward;

  project.track_a.steps[0] = Step(true, 0, 100, 1, 85, 112);
  project.track_a.steps[1] = Step(true, 1, 100, 1, 70, 100);
  project.track_a.steps[2] = Step(true, 2, 85, 2, 55, 96);
  project.track_a.steps[3] = Step(false, 3, 100, 1, 75, 100);
  project.track_a.steps[4] = Step(true, 4, 100, 1, 85, 110);
  project.track_a.steps[5] = Step(true, 5, 65, 3, 40, 92);
  project.track_a.steps[6] = Step(true, 6, 100, 1, 75, 106);
  project.track_a.steps[7] = Step(true, 7, 100, 1, 90, 112);

  project.track_b.steps[0] = Step(true, 0, 100, 1, 90, 92);
  project.track_b.steps[1] = Step(false, 2, 100, 1, 75, 88);
  project.track_b.steps[2] = Step(true, 4, 100, 1, 90, 92);
  project.track_b.steps[3] = Step(false, 2, 100, 1, 75, 88);
  project.track_b.steps[4] = Step(true, 5, 100, 2, 60, 96);

  load_project(project);
}

void App::set_random_seed(uint32_t seed) {
  sequencer_.set_random_seed(seed);
}

void App::start() {
  midi_.reset();
  midi_in_.reset();
  gates_.reset();
  monitor_queue_.clear();
  clock_.start();
  if (clock_.clock_source() == ClockSource::Internal) {
    midi_.send_start();
  }
  sequencer_.start();
  process_pending_engine_events();
}

void App::resume() {
  clock_.resume();
  if (clock_.clock_source() == ClockSource::Internal) {
    midi_.send_continue();
  }
  sequencer_.resume();
}

void App::stop() {
  sequencer_.stop();
  clock_.stop();
  if (clock_.clock_source() == ClockSource::Internal) {
    midi_.send_stop();
  }
  process_pending_engine_events();
}

void App::tick_internal() {
  if (clock_.clock_source() != ClockSource::Internal ||
      transport_state() != TransportState::Playing) {
    return;
  }

  sequencer_.tick();
  if ((sequencer_.current_tick() % kInternalTicksPerMidiClock) == 0U) {
    midi_.send_clock();
  }
  process_pending_engine_events();
}

ProjectState App::build_default_project() {
  ProjectState project{};
  project.tempo_bpm_x10 = 1200;
  project.root_note = 0;
  project.scale_id = 0;
  project.play_mode = PlayMode::Forward;
  project.machine_mode = MachineMode::Dual;
  project.track_a.length = 8;
  project.track_b.length = 8;
  project.track_a.midi_channel = 1;
  project.track_b.midi_channel = 2;
  return project;
}

void App::process_pending_engine_events() {
  EngineEvent event;
  while (sequencer_.pop_event(event)) {
    monitor_queue_.push(event);

    switch (event.type) {
      case EventType::NoteOn:
        midi_.send_note_on(event.midi_channel, event.note, event.velocity);
        break;
      case EventType::NoteOff:
        midi_.send_note_off(event.midi_channel, event.note);
        break;
      case EventType::GateHigh:
        gates_.set_gate(event.track, true);
        break;
      case EventType::GateLow:
        gates_.set_gate(event.track, false);
        break;
    }
  }
}

void App::receive_midi_byte(uint8_t byte) {
  midi_in_.feed_byte(byte);
  process_pending_midi_input_events();
}

void App::process_pending_midi_input_events() {
  MidiInputEvent event;
  while (midi_in_.pop_event(event)) {
    switch (event.type) {
      case MidiInputEventType::Clock:
        if (clock_.clock_source() == ClockSource::ExternalMidi &&
            transport_state() == TransportState::Playing) {
          advance_from_external_clock();
        }
        break;
      case MidiInputEventType::Start:
        if (clock_.clock_source() == ClockSource::ExternalMidi) {
          clock_.start();
          sequencer_.start();
          process_pending_engine_events();
        }
        break;
      case MidiInputEventType::Continue:
        if (clock_.clock_source() == ClockSource::ExternalMidi) {
          clock_.resume();
          sequencer_.resume();
        }
        break;
      case MidiInputEventType::Stop:
        if (clock_.clock_source() == ClockSource::ExternalMidi) {
          sequencer_.stop();
          clock_.stop();
          process_pending_engine_events();
        }
        break;
    }
  }
}

void App::advance_from_external_clock() {
  const uint8_t ticks = clock_.external_clock_to_internal_ticks();
  for (uint8_t index = 0; index < ticks; ++index) {
    sequencer_.tick();
    process_pending_engine_events();
  }
}

}  // namespace dixpas
