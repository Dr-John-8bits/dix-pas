#include "dixpas/display_engine.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace dixpas {

namespace {

const char* machine_mode_label(MachineMode mode) {
  return mode == MachineMode::Chain20 ? "C20" : "Dual";
}

const char* transport_label(TransportState state) {
  return state == TransportState::Playing ? "Play" : "Stop";
}

const char* track_label(TrackId track) {
  return track == TrackId::A ? "A" : "B";
}

const char* row3_label(Row3BaseMode mode) {
  return mode == Row3BaseMode::OnOff ? "OnOff" : "Prob";
}

const char* encoder_label(EncoderTarget target) {
  switch (target) {
    case EncoderTarget::Gate:
      return "Gate";
    case EncoderTarget::Velocity:
      return "Vel";
    case EncoderTarget::Note:
    default:
      return "Note";
  }
}

const char* global_target_label(GlobalTarget target) {
  switch (target) {
    case GlobalTarget::Root:
      return "Root";
    case GlobalTarget::Scale:
      return "Scale";
    case GlobalTarget::PlayMode:
      return "Play";
    case GlobalTarget::TrackLength:
      return "Length";
    case GlobalTarget::MidiChannel:
      return "MIDI";
    case GlobalTarget::PresetSlot:
      return "Preset";
    case GlobalTarget::Tempo:
    default:
      return "Tempo";
  }
}

const char* play_mode_label(PlayMode mode) {
  switch (mode) {
    case PlayMode::Reverse:
      return "Rev";
    case PlayMode::PingPong:
      return "Ping";
    case PlayMode::Random:
      return "Rand";
    case PlayMode::Forward:
    default:
      return "Fwd";
  }
}

const char* scale_label(uint8_t scale_id) {
  switch (scale_id % 4U) {
    case 1:
      return "Minor";
    case 2:
      return "Penta";
    case 3:
      return "Chrom";
    case 0:
    default:
      return "Major";
  }
}

const char* note_name(uint8_t note) {
  static constexpr const char* kNoteNames[12] = {
      "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
  };
  return kNoteNames[note % 12U];
}

void copy_line(char* dst, const char* src) {
  strncpy(dst, src, kDisplayLineWidth);
  dst[kDisplayLineWidth] = '\0';
}

void format_line(char* dst, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf(dst, kDisplayLineWidth + 1U, format, args);
  va_end(args);
}

void clear_frame(DisplayFrame& frame) {
  for (size_t line = 0; line < kDisplayLineCount; ++line) {
    frame.lines[line][0] = '\0';
  }
}

void render_status_line(const App& app, char* dst) {
  const ProjectState& project = app.project();
  if (app.clock_source() == ClockSource::ExternalMidi) {
    format_line(dst, "%s %s Ext Sync", machine_mode_label(project.machine_mode),
                transport_label(app.transport_state()));
    return;
  }

  format_line(dst, "%s %s Int %u.%u", machine_mode_label(project.machine_mode),
              transport_label(app.transport_state()),
              static_cast<unsigned>(project.tempo_bpm_x10 / 10U),
              static_cast<unsigned>(project.tempo_bpm_x10 % 10U));
}

void render_track_line(const App& app, const UiController& ui, TrackId track, char* dst) {
  const ProjectState& project = app.project();
  const Track& track_state = track == TrackId::A ? project.track_a : project.track_b;
  const char focus_marker = ui.track_focus() == track ? '>' : ' ';
  const uint8_t selected_step = static_cast<uint8_t>(ui.selected_step(track) + 1U);
  const uint8_t playhead_step = app.has_playhead_step(track)
                                    ? static_cast<uint8_t>(app.playhead_step(track) + 1U)
                                    : 0U;

  format_line(dst, "%s%c s%02u p%02u l%02u c%02u", track_label(track), focus_marker,
              static_cast<unsigned>(selected_step), static_cast<unsigned>(playhead_step),
              static_cast<unsigned>(track_state.length),
              static_cast<unsigned>(track_state.midi_channel));
}

void render_home_footer(const App& app, const UiController& ui, char* dst) {
  format_line(dst, "R3:%s Enc:%s", row3_label(ui.row3_base_mode()),
              encoder_label(ui.encoder_target()));

  if (ui.page() == UiPage::GlobalEdit && ui.global_target() == GlobalTarget::PresetSlot &&
      ui.shift_held()) {
    format_line(dst, "P%u Shift=Save", static_cast<unsigned>(ui.preset_slot() + 1U));
  } else if (ui.page() == UiPage::GlobalEdit &&
             ui.global_target() == GlobalTarget::PresetSlot) {
    format_line(dst, "P%u BtnLd Sh+BtnSv", static_cast<unsigned>(ui.preset_slot() + 1U));
  } else if (ui.shift_held()) {
    format_line(dst, "Shift Ratchet P%u", static_cast<unsigned>(ui.preset_slot() + 1U));
  } else if (ui.page() == UiPage::GlobalEdit) {
    format_line(dst, "Global %s", global_target_label(ui.global_target()));
  } else if (app.project().machine_mode == MachineMode::Chain20) {
    format_line(dst, "Root:%s %s", note_name(app.project().root_note),
                scale_label(app.project().scale_id));
  }
}

void render_global_value_line(const App& app, const UiController& ui, char* dst) {
  const ProjectState& project = app.project();
  const Track& track =
      ui.track_focus() == TrackId::A ? project.track_a : project.track_b;

  switch (ui.global_target()) {
    case GlobalTarget::Tempo:
      format_line(dst, "Tempo %u.%u BPM",
                  static_cast<unsigned>(project.tempo_bpm_x10 / 10U),
                  static_cast<unsigned>(project.tempo_bpm_x10 % 10U));
      break;
    case GlobalTarget::Root:
      format_line(dst, "Root %s", note_name(project.root_note));
      break;
    case GlobalTarget::Scale:
      format_line(dst, "Scale %s", scale_label(project.scale_id));
      break;
    case GlobalTarget::PlayMode:
      format_line(dst, "Play %s", play_mode_label(project.play_mode));
      break;
    case GlobalTarget::TrackLength:
      format_line(dst, "%s Len %u", track_label(ui.track_focus()),
                  static_cast<unsigned>(track.length));
      break;
    case GlobalTarget::MidiChannel:
      format_line(dst, "%s MIDI Ch %u", track_label(ui.track_focus()),
                  static_cast<unsigned>(track.midi_channel));
      break;
    case GlobalTarget::PresetSlot:
      format_line(dst, "Preset Slot %u", static_cast<unsigned>(ui.preset_slot() + 1U));
      break;
  }
}

}  // namespace

void DisplayEngine::render(const App& app, const UiController& ui, DisplayFrame& frame) const {
  clear_frame(frame);

  if (ui.page() == UiPage::Boot) {
    copy_line(frame.lines[0], "DIX PAS");
    copy_line(frame.lines[1], "Initialisation");
    return;
  }

  const UiOverlay& overlay = ui.overlay();
  if (overlay.active) {
    copy_line(frame.lines[0], overlay.line1);
    copy_line(frame.lines[1], overlay.line2);
    render_status_line(app, frame.lines[2]);
    render_home_footer(app, ui, frame.lines[3]);
    return;
  }

  render_status_line(app, frame.lines[0]);

  if (ui.page() == UiPage::GlobalEdit) {
    format_line(frame.lines[1], "Global %s", global_target_label(ui.global_target()));
    render_global_value_line(app, ui, frame.lines[2]);
    render_home_footer(app, ui, frame.lines[3]);
    return;
  }

  render_track_line(app, ui, TrackId::A, frame.lines[1]);
  render_track_line(app, ui, TrackId::B, frame.lines[2]);
  render_home_footer(app, ui, frame.lines[3]);
}

bool DisplayEngine::equals(const DisplayFrame& lhs, const DisplayFrame& rhs) {
  return memcmp(&lhs, &rhs, sizeof(DisplayFrame)) == 0;
}

}  // namespace dixpas
