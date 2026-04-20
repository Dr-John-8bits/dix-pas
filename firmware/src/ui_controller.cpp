#include "dixpas/ui_controller.hpp"

#include <stdio.h>

#include "dixpas/music_scales.hpp"
#include "dixpas/storage_engine.hpp"

namespace dixpas {

namespace {

constexpr uint8_t kProbabilityCycle[] = {0U, 25U, 50U, 75U, 100U};
constexpr uint8_t kMaxDegreeValue = 63U;
constexpr uint8_t kVelocityStep = 4U;
constexpr uint8_t kGateStep = 5U;
constexpr uint8_t kHardwareTestNoteA = 48U;
constexpr uint8_t kHardwareTestNoteB = 60U;
constexpr uint8_t kHardwareTestVelocity = 100U;

const char* track_name(TrackId track) {
  return track == TrackId::A ? "Track A" : "Track B";
}

const char* play_mode_name(PlayMode mode) {
  switch (mode) {
    case PlayMode::Forward:
      return "Forward";
    case PlayMode::Reverse:
      return "Reverse";
    case PlayMode::PingPong:
      return "PingPong";
    case PlayMode::Random:
      return "Random";
  }

  return "Forward";
}

const char* encoder_target_name(EncoderTarget target) {
  switch (target) {
    case EncoderTarget::Note:
      return "Note";
    case EncoderTarget::Gate:
      return "Gate";
    case EncoderTarget::Velocity:
      return "Velocity";
  }

  return "Note";
}

const char* global_target_name(GlobalTarget target) {
  switch (target) {
    case GlobalTarget::Tempo:
      return "Tempo";
    case GlobalTarget::ClockSource:
      return "Clock";
    case GlobalTarget::MachineMode:
      return "Mode";
    case GlobalTarget::Root:
      return "Root";
    case GlobalTarget::Scale:
      return "Scale";
    case GlobalTarget::PlayMode:
      return "Play Mode";
    case GlobalTarget::TrackLength:
      return "Track Length";
    case GlobalTarget::MidiChannel:
      return "MIDI Ch";
    case GlobalTarget::PresetSlot:
      return "Preset";
    case GlobalTarget::GenerativeSlot:
      return "Generative";
  }

  return "Global";
}

uint8_t wrap_increment(uint8_t value, uint8_t limit) {
  return static_cast<uint8_t>((value + 1U) % limit);
}

TrackId hardware_test_track(uint8_t phase) {
  return phase < 2U ? TrackId::A : TrackId::B;
}

bool hardware_test_phase_high(uint8_t phase) {
  return (phase % 2U) == 0U;
}

uint8_t hardware_test_note(TrackId track) {
  return track == TrackId::A ? kHardwareTestNoteA : kHardwareTestNoteB;
}

void format_step_note_value(const ProjectState& project, const Track& track, const Step& step,
                            char* dst, size_t size) {
  const uint8_t midi_note =
      resolve_scale_note(project.root_note, project.scale_id, step.degree, track.octave_offset);
  const int8_t octave = static_cast<int8_t>(midi_note / 12U) - 1;
  snprintf(dst, size, "Deg %u %s%d", static_cast<unsigned>(step.degree), note_name(midi_note),
           static_cast<int>(octave));
}

}  // namespace

UiController::UiController(App& app, StorageEngine* storage)
    : app_(app), storage_(storage) {
  reset();
}

void UiController::reset() {
  project_ = app_.project();
  page_ = UiPage::Home;
  track_focus_ = TrackId::A;
  selected_step_a_ = 0U;
  selected_step_b_ = 0U;
  row3_base_mode_ = Row3BaseMode::Probability;
  encoder_target_ = EncoderTarget::Note;
  global_target_ = GlobalTarget::Tempo;
  shift_held_ = false;
  preset_slot_ = 0U;
  generative_slot_ = 0U;
  overlay_ = {};
  overlay_timeout_ms_ = 0U;
  snprintf(diagnostic_event_, sizeof(diagnostic_event_), "Ready");
  hardware_test_mode_ = HardwareTestMode::Combined;
  hardware_test_running_ = false;
  hardware_test_phase_ = 0U;
  hardware_test_phase_elapsed_ms_ = 0U;
  snprintf(hardware_test_status_, sizeof(hardware_test_status_), "Idle");
  restore_preset_slot_from_metadata();
}

void UiController::enter_boot_page() {
  page_ = UiPage::Boot;
  overlay_.active = false;
  overlay_timeout_ms_ = 0U;
}

void UiController::leave_boot_page() {
  if (page_ == UiPage::Boot) {
    page_ = UiPage::Home;
    if (startup_message_pending_) {
      show_global_overlay("Startup", startup_message_);
      startup_message_pending_ = false;
    }
  }
}

void UiController::set_hardware_status(bool storage_available, bool display_available) {
  storage_available_ = storage_available;
  display_available_ = display_available;
}

void UiController::set_startup_message(const char* message) {
  startup_message_pending_ = true;
  snprintf(startup_message_, sizeof(startup_message_), "%s", message == nullptr ? "" : message);
}

void UiController::show_system_message(const char* title, const char* value) {
  show_global_overlay(title, value);
}

void UiController::update(uint16_t elapsed_ms) {
  if (page_ == UiPage::HardwareTest && hardware_test_running_) {
    uint32_t remaining = elapsed_ms;
    while (remaining > 0U) {
      const uint16_t budget =
          static_cast<uint16_t>(kHardwareTestPhaseDurationMs - hardware_test_phase_elapsed_ms_);
      if (remaining < budget) {
        hardware_test_phase_elapsed_ms_ =
            static_cast<uint16_t>(hardware_test_phase_elapsed_ms_ + remaining);
        remaining = 0U;
        break;
      }

      remaining -= budget;
      hardware_test_phase_elapsed_ms_ = 0U;
      advance_hardware_test_phase();
    }
  }

  if (!overlay_.active) {
    return;
  }

  if (elapsed_ms >= overlay_timeout_ms_) {
    overlay_.active = false;
    overlay_timeout_ms_ = 0U;
    if (page_ == UiPage::StepEdit) {
      page_ = UiPage::Home;
    }
    return;
  }

  overlay_timeout_ms_ = static_cast<uint16_t>(overlay_timeout_ms_ - elapsed_ms);
}

void UiController::attach_storage(StorageEngine& storage) {
  storage_ = &storage;
  restore_preset_slot_from_metadata();
}

void UiController::press_track_step(TrackId track, uint8_t index) {
  if (index >= kStepsPerTrack) {
    return;
  }

  if (page_ == UiPage::HardwareTest) {
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    char value[32];
    snprintf(value, sizeof(value), "%c Step %u", track == TrackId::A ? 'A' : 'B',
             static_cast<unsigned>(index + 1U));
    set_diagnostic_event(value);
    return;
  }

  track_focus_ = track;
  if (track == TrackId::A) {
    selected_step_a_ = index;
  } else {
    selected_step_b_ = index;
  }

  page_ = UiPage::StepEdit;

  char value[32];
  const Track& selected_track = (track == TrackId::A ? project_.track_a : project_.track_b);
  const Step& step = selected_track.steps[index];
  format_step_note_value(project_, selected_track, step, value, sizeof(value));
  show_step_overlay(track, index, "Selected", value);
}

void UiController::press_row3(uint8_t index) {
  if (index >= kStepsPerTrack) {
    return;
  }

  if (page_ == UiPage::HardwareTest) {
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    char value[32];
    snprintf(value, sizeof(value), "R3 Step %u", static_cast<unsigned>(index + 1U));
    set_diagnostic_event(value);
    return;
  }

  focused_selected_step() = index;
  Step& step = focused_track().steps[index];
  page_ = UiPage::StepEdit;

  char value[32];

  if (shift_held_) {
    cycle_ratchet(step);
    snprintf(value, sizeof(value), "x%u", static_cast<unsigned>(step.ratchet));
    commit_project(false);
    show_step_overlay(track_focus_, index, "Ratchet", value);
    return;
  }

  switch (row3_base_mode_) {
    case Row3BaseMode::Probability:
      cycle_probability(step);
      snprintf(value, sizeof(value), "%u%%", static_cast<unsigned>(step.probability));
      commit_project(false);
      show_step_overlay(track_focus_, index, "Probability", value);
      break;
    case Row3BaseMode::OnOff:
      step.active = !step.active;
      snprintf(value, sizeof(value), "%s", step.active ? "On" : "Off");
      commit_project(false);
      show_step_overlay(track_focus_, index, "Step", value);
      break;
  }
}

void UiController::rotate_encoder(int8_t delta) {
  if (delta == 0) {
    return;
  }

  if (page_ == UiPage::HardwareTest) {
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    char value[32];
    snprintf(value, sizeof(value), "Encoder %s%d", delta > 0 ? "+" : "",
             static_cast<int>(delta));
    set_diagnostic_event(value);
    return;
  }

  if (page_ == UiPage::GlobalEdit) {
    edit_global_value(delta);
    return;
  }

  page_ = UiPage::StepEdit;
  edit_step_value(delta);
}

void UiController::press_encoder_button() {
  if (page_ == UiPage::HardwareTest) {
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    set_diagnostic_event("Encoder Button");
    return;
  }

  if (page_ == UiPage::GlobalEdit) {
    if (global_target_ == GlobalTarget::PresetSlot) {
      if (shift_held_) {
        save_selected_preset();
      } else {
        load_selected_preset();
      }
      return;
    }
    if (global_target_ == GlobalTarget::GenerativeSlot) {
      if (shift_held_) {
        mutate_selected_generative_slot();
      } else {
        apply_selected_generative_slot();
      }
      return;
    }

    global_target_ =
        static_cast<GlobalTarget>(wrap_increment(static_cast<uint8_t>(global_target_), 10U));
    show_global_overlay(global_target_name(global_target_), "Selected");
    return;
  }

  encoder_target_ =
      static_cast<EncoderTarget>(wrap_increment(static_cast<uint8_t>(encoder_target_), 3U));
  show_step_overlay(track_focus_, focused_selected_step(), "Encoder",
                    encoder_target_name(encoder_target_));
}

void UiController::press_mode_short() {
  if (page_ == UiPage::HardwareTest) {
    hardware_test_mode_ =
        static_cast<HardwareTestMode>(wrap_increment(static_cast<uint8_t>(hardware_test_mode_), 3U));
    restart_hardware_test();
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    set_diagnostic_event("Mode Short");
    return;
  }

  if (page_ == UiPage::GlobalEdit) {
    global_target_ =
        static_cast<GlobalTarget>(wrap_increment(static_cast<uint8_t>(global_target_), 10U));
    show_global_overlay(global_target_name(global_target_), "Selected");
    return;
  }

  row3_base_mode_ = row3_base_mode_ == Row3BaseMode::Probability ? Row3BaseMode::OnOff
                                                                  : Row3BaseMode::Probability;
  show_global_overlay("Row 3",
                      row3_base_mode_ == Row3BaseMode::Probability ? "Probability" : "On/Off");
}

void UiController::press_mode_long() {
  if (page_ == UiPage::HardwareTest) {
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    set_diagnostic_event("Mode Long");
    return;
  }

  if (page_ == UiPage::GlobalEdit) {
    page_ = UiPage::Home;
    overlay_.active = false;
    overlay_timeout_ms_ = 0U;
    return;
  }

  page_ = UiPage::GlobalEdit;
  show_global_overlay(global_target_name(global_target_), "Edit");
}

void UiController::set_shift_held(bool held) {
  shift_held_ = held;
  if (page_ == UiPage::HardwareTest) {
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    set_diagnostic_event(held ? "Shift On" : "Shift Off");
    return;
  }

  if (held) {
    if (page_ == UiPage::GlobalEdit && global_target_ == GlobalTarget::PresetSlot) {
      show_global_overlay("Shift", "Save");
    } else if (page_ == UiPage::GlobalEdit && global_target_ == GlobalTarget::GenerativeSlot) {
      show_global_overlay("Shift", "Mutate");
    } else {
      show_global_overlay("Shift", "Ratchet");
    }
  }
}

void UiController::press_play() {
  if (shift_held_) {
    toggle_hardware_test_page();
    return;
  }

  if (page_ == UiPage::HardwareTest) {
    start_hardware_test();
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    set_diagnostic_event("Play Button");
    return;
  }

  app_.start();
  show_transport_overlay("Play");
}

void UiController::press_stop() {
  if (page_ == UiPage::HardwareTest) {
    stop_hardware_test();
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    set_diagnostic_event("Stop Button");
    return;
  }

  app_.stop();
  show_transport_overlay("Stop");
}

void UiController::press_reset() {
  if (shift_held_) {
    if (page_ == UiPage::HardwareTest) {
      stop_hardware_test();
    }
    toggle_diagnostic_page();
    return;
  }

  if (page_ == UiPage::HardwareTest) {
    restart_hardware_test();
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    set_diagnostic_event("Reset Button");
    return;
  }

  app_.reset_playhead();
  show_transport_overlay("Reset");
}

uint8_t UiController::selected_step(TrackId track) const {
  return track == TrackId::A ? selected_step_a_ : selected_step_b_;
}

void UiController::commit_project(bool reset_playhead) {
  app_.apply_project(project_, reset_playhead);
  project_ = app_.project();
}

void UiController::show_step_overlay(TrackId track, uint8_t step_index, const char* parameter,
                                     const char* value) {
  snprintf(overlay_.line1, sizeof(overlay_.line1), "%s - Step %u", track_name(track),
           static_cast<unsigned>(step_index + 1U));
  snprintf(overlay_.line2, sizeof(overlay_.line2), "%s: %s", parameter, value);
  overlay_.active = true;
  overlay_timeout_ms_ = kOverlayTimeoutMs;
}

void UiController::show_global_overlay(const char* parameter, const char* value) {
  snprintf(overlay_.line1, sizeof(overlay_.line1), "Global");
  snprintf(overlay_.line2, sizeof(overlay_.line2), "%s: %s", parameter, value);
  overlay_.active = true;
  overlay_timeout_ms_ = kOverlayTimeoutMs;
}

void UiController::show_transport_overlay(const char* label) {
  snprintf(overlay_.line1, sizeof(overlay_.line1), "Transport");
  snprintf(overlay_.line2, sizeof(overlay_.line2), "%s", label);
  overlay_.active = true;
  overlay_timeout_ms_ = kOverlayTimeoutMs;
}

void UiController::load_selected_preset() {
  if (storage_ == nullptr) {
    show_global_overlay("Preset", "No storage");
    return;
  }

  ProjectState loaded_project{};
  const StorageStatus status = storage_->load_preset(preset_slot_, loaded_project);
  if (status != StorageStatus::Ok) {
    show_storage_overlay("Load", status);
    return;
  }

  project_ = loaded_project;
  app_.load_project(project_);
  project_ = app_.project();
  update_storage_metadata(false);

  char value[32];
  snprintf(value, sizeof(value), "Loaded Slot %u", static_cast<unsigned>(preset_slot_ + 1U));
  show_global_overlay("Preset", value);
}

void UiController::save_selected_preset() {
  if (storage_ == nullptr) {
    show_global_overlay("Preset", "No storage");
    return;
  }

  const StorageStatus status = storage_->save_preset(preset_slot_, project_);
  if (status != StorageStatus::Ok) {
    show_storage_overlay("Save", status);
    return;
  }

  update_storage_metadata(true);

  char value[32];
  snprintf(value, sizeof(value), "Saved Slot %u", static_cast<unsigned>(preset_slot_ + 1U));
  show_global_overlay("Preset", value);
}

void UiController::apply_selected_generative_slot() {
  if (!app_.has_generative_slot(generative_slot_)) {
    show_global_overlay("Generative", "Empty slot");
    return;
  }

  app_.apply_generative_slot(generative_slot_, true);
  project_ = app_.project();

  char value[32];
  snprintf(value, sizeof(value), "Applied G%u", static_cast<unsigned>(generative_slot_ + 1U));
  show_global_overlay("Generative", value);
}

void UiController::mutate_selected_generative_slot() {
  if (!app_.has_generative_slot(generative_slot_)) {
    show_global_overlay("Generative", "Empty slot");
    return;
  }

  app_.mutate_generative_slot(generative_slot_, false);
  project_ = app_.project();

  char value[32];
  snprintf(value, sizeof(value), "Mutated G%u", static_cast<unsigned>(generative_slot_ + 1U));
  show_global_overlay("Generative", value);
}

void UiController::update_storage_metadata(bool saved) {
  if (storage_ == nullptr) {
    return;
  }

  StorageMetadataV1 metadata{};
  if (storage_->load_metadata(metadata) != StorageStatus::Ok) {
    metadata = StorageEngine::build_default_metadata();
  }

  if (saved) {
    metadata.last_saved_slot = preset_slot_;
  } else {
    metadata.last_loaded_slot = preset_slot_;
  }

  storage_->save_metadata(metadata);
}

void UiController::show_storage_overlay(const char* action, StorageStatus status) {
  char value[32];

  switch (status) {
    case StorageStatus::Ok:
      snprintf(value, sizeof(value), "%s OK", action);
      break;
    case StorageStatus::InvalidMagic:
      snprintf(value, sizeof(value), "Empty Slot %u", static_cast<unsigned>(preset_slot_ + 1U));
      break;
    case StorageStatus::InvalidVersion:
      snprintf(value, sizeof(value), "Version Error");
      break;
    case StorageStatus::InvalidPayloadSize:
      snprintf(value, sizeof(value), "Format Error");
      break;
    case StorageStatus::InvalidCrc:
      snprintf(value, sizeof(value), "CRC Error");
      break;
    case StorageStatus::IoError:
      snprintf(value, sizeof(value), "I/O Error");
      break;
    case StorageStatus::InvalidSlot:
    default:
      snprintf(value, sizeof(value), "Bad Slot");
      break;
  }

  show_global_overlay("Preset", value);
}

void UiController::restore_preset_slot_from_metadata() {
  if (storage_ == nullptr) {
    return;
  }

  StorageMetadataV1 metadata{};
  if (storage_->load_metadata(metadata) != StorageStatus::Ok) {
    return;
  }

  uint8_t slot = 0U;
  if (StorageEngine::preferred_startup_slot(metadata, slot)) {
    preset_slot_ = slot;
  }
}

void UiController::set_diagnostic_event(const char* value) {
  snprintf(diagnostic_event_, sizeof(diagnostic_event_), "%s", value == nullptr ? "" : value);
}

void UiController::toggle_diagnostic_page() {
  overlay_.active = false;
  overlay_timeout_ms_ = 0U;
  if (page_ == UiPage::Diagnostic) {
    page_ = UiPage::Home;
    set_diagnostic_event("Diag Exit");
    return;
  }

  page_ = UiPage::Diagnostic;
  set_diagnostic_event("Diag Enter");
}

void UiController::toggle_hardware_test_page() {
  overlay_.active = false;
  overlay_timeout_ms_ = 0U;

  if (page_ == UiPage::HardwareTest) {
    stop_hardware_test();
    page_ = UiPage::Home;
    show_transport_overlay("HW Test Off");
    return;
  }

  if (page_ == UiPage::Diagnostic) {
    page_ = UiPage::Home;
  }

  if (app_.transport_state() == TransportState::Playing) {
    app_.stop();
  }
  app_.clear_manual_test_outputs();
  page_ = UiPage::HardwareTest;
  hardware_test_mode_ = HardwareTestMode::Combined;
  start_hardware_test();
}

void UiController::start_hardware_test() {
  if (page_ != UiPage::HardwareTest) {
    page_ = UiPage::HardwareTest;
  }

  overlay_.active = false;
  overlay_timeout_ms_ = 0U;
  hardware_test_running_ = true;
  hardware_test_phase_ = 0U;
  hardware_test_phase_elapsed_ms_ = 0U;
  apply_hardware_test_phase();
}

void UiController::stop_hardware_test() {
  hardware_test_running_ = false;
  hardware_test_phase_elapsed_ms_ = 0U;
  app_.clear_manual_test_outputs();
  snprintf(hardware_test_status_, sizeof(hardware_test_status_), "Hold Low");
}

void UiController::restart_hardware_test() {
  if (page_ != UiPage::HardwareTest) {
    return;
  }

  start_hardware_test();
}

void UiController::advance_hardware_test_phase() {
  hardware_test_phase_ = static_cast<uint8_t>((hardware_test_phase_ + 1U) % 4U);
  apply_hardware_test_phase();
}

void UiController::apply_hardware_test_phase() {
  const TrackId track = hardware_test_track(hardware_test_phase_);
  const bool high = hardware_test_phase_high(hardware_test_phase_);
  const uint8_t midi_channel =
      track == TrackId::A ? project_.track_a.midi_channel : project_.track_b.midi_channel;
  const uint8_t note = hardware_test_note(track);
  const int8_t octave = static_cast<int8_t>(note / 12U) - 1;

  app_.clear_manual_test_outputs();

  if (high) {
    if (hardware_test_mode_ != HardwareTestMode::GateOnly) {
      app_.set_manual_note(track, midi_channel, note, kHardwareTestVelocity, true);
    }
    if (hardware_test_mode_ != HardwareTestMode::MidiOnly) {
      app_.set_manual_gate(track, true);
    }
  }

  if (hardware_test_mode_ == HardwareTestMode::GateOnly) {
    snprintf(hardware_test_status_, sizeof(hardware_test_status_), "%c %s Gate",
             track == TrackId::A ? 'A' : 'B', high ? "On" : "Off");
    return;
  }

  snprintf(hardware_test_status_, sizeof(hardware_test_status_), "%c %s Ch%02u %s%d",
           track == TrackId::A ? 'A' : 'B', high ? "On" : "Off",
           static_cast<unsigned>(midi_channel), note_name(note), static_cast<int>(octave));
}

Track& UiController::focused_track() {
  return track_focus_ == TrackId::A ? project_.track_a : project_.track_b;
}

const Track& UiController::focused_track() const {
  return track_focus_ == TrackId::A ? project_.track_a : project_.track_b;
}

uint8_t& UiController::focused_selected_step() {
  return track_focus_ == TrackId::A ? selected_step_a_ : selected_step_b_;
}

uint8_t UiController::focused_selected_step() const {
  return track_focus_ == TrackId::A ? selected_step_a_ : selected_step_b_;
}

Step& UiController::selected_step_ref() {
  return focused_track().steps[focused_selected_step()];
}

void UiController::cycle_probability(Step& step) {
  for (size_t index = 0; index < (sizeof(kProbabilityCycle) / sizeof(kProbabilityCycle[0])); ++index) {
    if (step.probability == kProbabilityCycle[index]) {
      step.probability =
          kProbabilityCycle[(index + 1U) % (sizeof(kProbabilityCycle) / sizeof(kProbabilityCycle[0]))];
      return;
    }
  }

  step.probability = kProbabilityCycle[0];
}

void UiController::cycle_ratchet(Step& step) {
  step.ratchet = step.ratchet >= kMaxRatchet ? kMinRatchet : static_cast<uint8_t>(step.ratchet + 1U);
}

void UiController::edit_step_value(int8_t delta) {
  Step& step = selected_step_ref();
  char value[32];

  switch (encoder_target_) {
    case EncoderTarget::Note: {
      int16_t degree = static_cast<int16_t>(step.degree) + delta;
      if (degree < 0) {
        degree = 0;
      }
      if (degree > kMaxDegreeValue) {
        degree = kMaxDegreeValue;
      }
      step.degree = static_cast<uint8_t>(degree);
      format_step_note_value(project_, focused_track(), step, value, sizeof(value));
      commit_project(false);
      show_step_overlay(track_focus_, focused_selected_step(), "Degree", value);
      break;
    }
    case EncoderTarget::Gate: {
      int16_t gate = static_cast<int16_t>(step.gate) + delta * static_cast<int8_t>(kGateStep);
      if (gate < kMinGatePercent) {
        gate = kMinGatePercent;
      }
      if (gate > kMaxGatePercent) {
        gate = kMaxGatePercent;
      }
      step.gate = static_cast<uint8_t>(gate);
      snprintf(value, sizeof(value), "%u%%", static_cast<unsigned>(step.gate));
      commit_project(false);
      show_step_overlay(track_focus_, focused_selected_step(), "Gate", value);
      break;
    }
    case EncoderTarget::Velocity: {
      int16_t velocity = static_cast<int16_t>(step.velocity) + delta * static_cast<int8_t>(kVelocityStep);
      if (velocity < 1) {
        velocity = 1;
      }
      if (velocity > 127) {
        velocity = 127;
      }
      step.velocity = static_cast<uint8_t>(velocity);
      snprintf(value, sizeof(value), "%u", static_cast<unsigned>(step.velocity));
      commit_project(false);
      show_step_overlay(track_focus_, focused_selected_step(), "Velocity", value);
      break;
    }
  }
}

void UiController::edit_global_value(int8_t delta) {
  if (delta == 0) {
    return;
  }

  char value[32];
  bool reset_playhead = false;

  switch (global_target_) {
    case GlobalTarget::Tempo: {
      int32_t tempo = static_cast<int32_t>(project_.tempo_bpm_x10) + static_cast<int32_t>(delta) * 10;
      if (tempo < 300) {
        tempo = 300;
      }
      if (tempo > 3000) {
        tempo = 3000;
      }
      project_.tempo_bpm_x10 = static_cast<uint16_t>(tempo);
      snprintf(value, sizeof(value), "%u.%u BPM",
               static_cast<unsigned>(project_.tempo_bpm_x10 / 10U),
               static_cast<unsigned>(project_.tempo_bpm_x10 % 10U));
      break;
    }
    case GlobalTarget::ClockSource: {
      ClockSource source = app_.clock_source();
      source = source == ClockSource::Internal ? ClockSource::ExternalMidi : ClockSource::Internal;
      app_.set_clock_source(source);
      snprintf(value, sizeof(value), "%s",
               source == ClockSource::ExternalMidi ? "External" : "Internal");
      break;
    }
    case GlobalTarget::MachineMode: {
      project_.machine_mode = project_.machine_mode == MachineMode::Dual ? MachineMode::Chain20
                                                                         : MachineMode::Dual;
      reset_playhead = true;
      snprintf(value, sizeof(value), "%s",
               project_.machine_mode == MachineMode::Chain20 ? "Chain20" : "Dual");
      break;
    }
    case GlobalTarget::Root: {
      int16_t root = static_cast<int16_t>(project_.root_note) + delta;
      while (root < 0) {
        root += 12;
      }
      while (root >= 12) {
        root -= 12;
      }
      project_.root_note = static_cast<uint8_t>(root);
      snprintf(value, sizeof(value), "%s", note_name(project_.root_note));
      break;
    }
    case GlobalTarget::Scale: {
      int16_t scale = static_cast<int16_t>(project_.scale_id) + delta;
      while (scale < 0) {
        scale += kScaleCount;
      }
      while (scale >= kScaleCount) {
        scale -= kScaleCount;
      }
      project_.scale_id = static_cast<uint8_t>(scale);
      snprintf(value, sizeof(value), "%s", scale_name(project_.scale_id));
      break;
    }
    case GlobalTarget::PlayMode: {
      int16_t mode = static_cast<int16_t>(project_.play_mode) + delta;
      while (mode < 0) {
        mode += 4;
      }
      while (mode >= 4) {
        mode -= 4;
      }
      project_.play_mode = static_cast<PlayMode>(mode);
      reset_playhead = true;
      snprintf(value, sizeof(value), "%s", play_mode_name(project_.play_mode));
      break;
    }
    case GlobalTarget::TrackLength: {
      Track& track = focused_track();
      int16_t length = static_cast<int16_t>(track.length) + delta;
      if (length < kMinTrackLength) {
        length = kMinTrackLength;
      }
      if (length > kMaxTrackLength) {
        length = kMaxTrackLength;
      }
      track.length = static_cast<uint8_t>(length);
      reset_playhead = true;
      snprintf(value, sizeof(value), "%u", static_cast<unsigned>(track.length));
      break;
    }
    case GlobalTarget::MidiChannel: {
      Track& track = project_.machine_mode == MachineMode::Chain20 ? project_.track_a : focused_track();
      int16_t channel = static_cast<int16_t>(track.midi_channel) + delta;
      if (channel < kMinMidiChannel) {
        channel = kMinMidiChannel;
      }
      if (channel > kMaxMidiChannel) {
        channel = kMaxMidiChannel;
      }
      track.midi_channel = static_cast<uint8_t>(channel);
      if (project_.machine_mode == MachineMode::Chain20) {
        snprintf(value, sizeof(value), "Chain %u",
                 static_cast<unsigned>(project_.track_a.midi_channel));
      } else {
        snprintf(value, sizeof(value), "%u", static_cast<unsigned>(track.midi_channel));
      }
      break;
    }
    case GlobalTarget::PresetSlot: {
      int16_t slot = static_cast<int16_t>(preset_slot_) + delta;
      while (slot < 0) {
        slot += kStoragePresetSlotCount;
      }
      while (slot >= kStoragePresetSlotCount) {
        slot -= kStoragePresetSlotCount;
      }
      preset_slot_ = static_cast<uint8_t>(slot);
      snprintf(value, sizeof(value), "Slot %u", static_cast<unsigned>(preset_slot_ + 1U));
      break;
    }
    case GlobalTarget::GenerativeSlot: {
      int16_t slot = static_cast<int16_t>(generative_slot_) + delta;
      while (slot < 0) {
        slot += kGenerativeSlotCount;
      }
      while (slot >= kGenerativeSlotCount) {
        slot -= kGenerativeSlotCount;
      }
      generative_slot_ = static_cast<uint8_t>(slot);
      snprintf(value, sizeof(value), "G%u", static_cast<unsigned>(generative_slot_ + 1U));
      break;
    }
  }

  commit_project(reset_playhead);
  show_global_overlay(global_target_name(global_target_), value);
}

}  // namespace dixpas
