#include "dixpas/app.hpp"

namespace dixpas {

App::App() {
  seed_demo_generative_bank();
  load_project(build_default_project());
}

void App::load_project(const ProjectState& project) {
  apply_project(project, true);
}

void App::apply_project(const ProjectState& project, bool reset_playhead) {
  clock_.set_tempo_bpm_x10(project.tempo_bpm_x10);
  sequencer_.apply_project(project, reset_playhead);
  process_pending_engine_events();
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

bool App::load_generative_recipe(uint8_t slot, GenerativeRecipe& recipe) const {
  return generative_bank_.load(slot, recipe);
}

bool App::has_generative_slot(uint8_t slot) const {
  return generative_bank_.has_slot(slot);
}

void App::apply_generative_slot(uint8_t slot, bool reset_playhead) {
  GenerativeRecipe recipe{};
  if (!generative_bank_.load(slot, recipe)) {
    return;
  }

  apply_generative_recipe(recipe, reset_playhead);
}

void App::mutate_generative_slot(uint8_t slot, bool reset_playhead) {
  GenerativeRecipe recipe{};
  if (!generative_bank_.load(slot, recipe)) {
    return;
  }

  mutate_from_generative_recipe(recipe, reset_playhead);
}

void App::apply_generative_recipe(const GenerativeRecipe& recipe, bool reset_playhead) {
  ProjectState generated_project = project();
  generator_.apply_recipe(generated_project, recipe);
  apply_project(generated_project, reset_playhead);
}

void App::mutate_from_generative_recipe(const GenerativeRecipe& recipe, bool reset_playhead) {
  ProjectState mutated_project = project();
  generator_.mutate_project(mutated_project, recipe);
  apply_project(mutated_project, reset_playhead);
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

void App::reset_playhead() {
  sequencer_.reset_playhead();
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

void App::seed_demo_generative_bank() {
  generative_bank_.clear();
  generative_bank_.store(0, build_demo_recipe_dorian());
  generative_bank_.store(1, build_demo_recipe_chromatic());
}

GenerativeRecipe App::build_demo_recipe_dorian() {
  GenerativeRecipe recipe{};
  recipe.seed = 0x20260420U;
  recipe.root_note = 9U;   // A
  recipe.scale_id = 5U;    // Dorian
  recipe.machine_mode = MachineMode::Dual;
  recipe.play_mode = PlayMode::Forward;
  recipe.mutate_track_a_on_cycle = true;
  recipe.track_a.settings.rhythm.length = 10;
  recipe.track_a.settings.rhythm.pulses = 4;
  recipe.track_a.settings.rhythm.rotation = 1;
  recipe.track_a.settings.melody.contour = MelodyContour::RandomWalk;
  recipe.track_a.settings.melody.min_degree = 0;
  recipe.track_a.settings.melody.max_degree = 9;
  recipe.track_a.settings.melody.octave_offset = 0;
  recipe.track_a.settings.melody.probability = 100;
  recipe.track_a.settings.melody.gate = 75;
  recipe.track_a.settings.melody.velocity = 104;
  recipe.track_a.settings.mutation.enabled = true;
  recipe.track_a.settings.mutation.chance_percent = 100;
  recipe.track_a.settings.mutation.max_degree_delta = 2;
  recipe.track_a.settings.mutation.probability_jitter = 10;
  recipe.track_b.settings.rhythm.length = 10;
  recipe.track_b.settings.rhythm.pulses = 7;
  recipe.track_b.settings.rhythm.rotation = 0;
  recipe.track_b.settings.melody.contour = MelodyContour::Alternating;
  recipe.track_b.settings.melody.min_degree = 2;
  recipe.track_b.settings.melody.max_degree = 12;
  recipe.track_b.settings.melody.octave_offset = -1;
  recipe.track_b.settings.melody.probability = 90;
  recipe.track_b.settings.melody.gate = 60;
  recipe.track_b.settings.melody.velocity = 96;
  return recipe;
}

GenerativeRecipe App::build_demo_recipe_chromatic() {
  GenerativeRecipe recipe{};
  recipe.seed = 0x20260421U;
  recipe.root_note = 0U;   // C
  recipe.scale_id = 11U;   // Chromatic
  recipe.machine_mode = MachineMode::Chain20;
  recipe.play_mode = PlayMode::PingPong;
  recipe.track_a.settings.rhythm.length = 10;
  recipe.track_a.settings.rhythm.pulses = 5;
  recipe.track_a.settings.rhythm.rotation = 0;
  recipe.track_a.settings.melody.contour = MelodyContour::Ascending;
  recipe.track_a.settings.melody.min_degree = 0;
  recipe.track_a.settings.melody.max_degree = 11;
  recipe.track_a.settings.melody.octave_offset = 0;
  recipe.track_b.settings.rhythm.length = 10;
  recipe.track_b.settings.rhythm.pulses = 3;
  recipe.track_b.settings.rhythm.rotation = 2;
  recipe.track_b.settings.melody.contour = MelodyContour::Descending;
  recipe.track_b.settings.melody.min_degree = 0;
  recipe.track_b.settings.melody.max_degree = 11;
  recipe.track_b.settings.melody.octave_offset = 1;
  return recipe;
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
