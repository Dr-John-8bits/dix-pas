#include "dixpas/app.hpp"

namespace dixpas {

App::App() {
  load_project(build_default_project());
}

void App::load_project(const ProjectState& project) {
  clock_.set_tempo_bpm_x10(project.tempo_bpm_x10);
  sequencer_.load_project(project);
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
  clock_.start();
  sequencer_.start();
}

void App::stop() {
  sequencer_.stop();
  clock_.stop();
}

void App::tick() {
  sequencer_.tick();
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

}  // namespace dixpas
