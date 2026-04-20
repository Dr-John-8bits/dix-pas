#if defined(DIXPAS_NATIVE_CHECKS)

#include <cstdio>
#include <string.h>

#include "dixpas/app.hpp"
#include "dixpas/firmware_info.hpp"
#include "dixpas/fram_i2c_backend.hpp"
#include "dixpas/music_scales.hpp"
#include "dixpas/oled_display.hpp"
#include "dixpas/storage_engine.hpp"
#include "dixpas/ui_controller.hpp"
#include "dixpas/ui_scanner.hpp"

namespace {

using namespace dixpas;

struct CheckContext {
  const char* test_name = nullptr;
  bool failed = false;
};

class MockFramI2cPort : public FramI2cPort {
 public:
  bool begin() override {
    begin_called_ = true;
    return begin_ok_;
  }

  bool probe(uint8_t device_address) override {
    last_probe_address_ = device_address;
    return probe_ok_;
  }

  bool write_bytes(uint8_t device_address, const uint8_t* data, size_t size) override {
    last_device_address_ = device_address;
    if (size < 2U || data == nullptr) {
      return false;
    }

    if (size > max_write_payload_) {
      max_write_payload_ = size;
    }

    const uint16_t address =
        static_cast<uint16_t>((static_cast<uint16_t>(data[0]) << 8U) | data[1]);
    cursor_ = address;

    if (size == 2U) {
      return true;
    }

    const size_t payload_size = size - 2U;
    if ((static_cast<size_t>(address) + payload_size) > sizeof(bytes_)) {
      return false;
    }

    memcpy(bytes_ + address, data + 2U, payload_size);
    cursor_ = static_cast<uint16_t>(address + payload_size);
    ++data_write_count_;
    return true;
  }

  bool read_bytes(uint8_t device_address, uint8_t* dst, size_t size) override {
    last_device_address_ = device_address;
    if (dst == nullptr || (static_cast<size_t>(cursor_) + size) > sizeof(bytes_)) {
      return false;
    }

    if (size > max_read_payload_) {
      max_read_payload_ = size;
    }

    memcpy(dst, bytes_ + cursor_, size);
    cursor_ = static_cast<uint16_t>(cursor_ + size);
    ++data_read_count_;
    return true;
  }

  uint8_t max_write_payload() const { return max_write_payload_; }
  uint8_t max_read_payload() const { return max_read_payload_; }
  uint16_t data_write_count() const { return data_write_count_; }
  uint16_t data_read_count() const { return data_read_count_; }
  uint8_t last_probe_address() const { return last_probe_address_; }
  bool begin_called() const { return begin_called_; }

 private:
  uint8_t bytes_[kMemoryStorageCapacity]{};
  uint16_t cursor_ = 0U;
  uint8_t max_write_payload_ = 0U;
  uint8_t max_read_payload_ = 0U;
  uint16_t data_write_count_ = 0U;
  uint16_t data_read_count_ = 0U;
  uint8_t last_device_address_ = 0U;
  uint8_t last_probe_address_ = 0U;
  bool begin_called_ = false;
  bool begin_ok_ = true;
  bool probe_ok_ = true;
};

class MockOledI2cPort : public OledI2cPort {
 public:
  bool begin() override {
    begin_called_ = true;
    return begin_ok_;
  }

  bool write_bytes(uint8_t device_address, const uint8_t* data, size_t size) override {
    last_device_address_ = device_address;
    if (data == nullptr || size == 0U) {
      return false;
    }

    if (size > max_packet_size_) {
      max_packet_size_ = static_cast<uint8_t>(size);
    }
    if (data[0] == 0x40U) {
      ++data_packet_count_;
      for (size_t index = 1; index < size; ++index) {
        if (data[index] != 0U) {
          saw_non_zero_pixels_ = true;
          break;
        }
      }
    } else if (data[0] == 0x00U) {
      ++command_packet_count_;
    }

    return true;
  }

  bool begin_called() const { return begin_called_; }
  uint8_t last_device_address() const { return last_device_address_; }
  uint8_t max_packet_size() const { return max_packet_size_; }
  uint16_t command_packet_count() const { return command_packet_count_; }
  uint16_t data_packet_count() const { return data_packet_count_; }
  bool saw_non_zero_pixels() const { return saw_non_zero_pixels_; }

 private:
  bool begin_called_ = false;
  bool begin_ok_ = true;
  uint8_t last_device_address_ = 0U;
  uint8_t max_packet_size_ = 0U;
  uint16_t command_packet_count_ = 0U;
  uint16_t data_packet_count_ = 0U;
  bool saw_non_zero_pixels_ = false;
};

void fail(CheckContext& ctx, const char* file, int line, const char* expression) {
  std::printf("  FAIL %s:%d -> %s\n", file, line, expression);
  ctx.failed = true;
}

#define CHECK_TRUE(ctx, expr)        \
  do {                               \
    if (!(expr)) {                   \
      fail(ctx, __FILE__, __LINE__, #expr); \
      return false;                  \
    }                                \
  } while (false)

#define CHECK_EQ(ctx, expected, actual) \
  do {                                  \
    if ((expected) != (actual)) {       \
      fail(ctx, __FILE__, __LINE__, #actual " == " #expected); \
      return false;                     \
    }                                   \
  } while (false)

bool steps_equal(const Step& lhs, const Step& rhs) {
  return lhs.active == rhs.active && lhs.degree == rhs.degree &&
         lhs.probability == rhs.probability && lhs.ratchet == rhs.ratchet &&
         lhs.gate == rhs.gate && lhs.velocity == rhs.velocity;
}

bool tracks_equal(const Track& lhs, const Track& rhs) {
  if (lhs.length != rhs.length || lhs.midi_channel != rhs.midi_channel ||
      lhs.octave_offset != rhs.octave_offset) {
    return false;
  }

  for (uint8_t index = 0; index < kStepsPerTrack; ++index) {
    if (!steps_equal(lhs.steps[index], rhs.steps[index])) {
      return false;
    }
  }

  return true;
}

bool projects_equal(const ProjectState& lhs, const ProjectState& rhs) {
  return tracks_equal(lhs.track_a, rhs.track_a) && tracks_equal(lhs.track_b, rhs.track_b) &&
         lhs.machine_mode == rhs.machine_mode && lhs.tempo_bpm_x10 == rhs.tempo_bpm_x10 &&
         lhs.root_note == rhs.root_note && lhs.scale_id == rhs.scale_id &&
         lhs.play_mode == rhs.play_mode;
}

uint8_t count_active_steps(const Track& track) {
  uint8_t count = 0U;
  for (uint8_t index = 0; index < track.length; ++index) {
    if (track.steps[index].active) {
      ++count;
    }
  }
  return count;
}

void cycle_to_global_target(UiController& ui, GlobalTarget target) {
  uint8_t attempts = 0U;
  while (ui.global_target() != target && attempts < 16U) {
    ui.press_mode_short();
    ++attempts;
  }
}

bool test_scale_registry_and_quantization(CheckContext& ctx) {
  CHECK_EQ(ctx, 12U, kScaleCount);
  CHECK_TRUE(ctx, scale_name(11U) != nullptr);
  CHECK_TRUE(ctx, scale_name(1U) != nullptr);
  CHECK_EQ(ctx, 60U, resolve_scale_note(0U, 11U, 0U, 0));
  CHECK_EQ(ctx, 61U, resolve_scale_note(0U, 11U, 1U, 0));
  CHECK_EQ(ctx, 72U, resolve_scale_note(0U, 11U, 12U, 0));
  CHECK_EQ(ctx, 63U, resolve_scale_note(0U, 1U, 2U, 0));
  return true;
}

bool test_storage_roundtrip_preserves_project(CheckContext& ctx) {
  App app;
  MemoryStorageBackend backend;
  StorageEngine storage(backend);

  app.seed_demo_project();
  const ProjectState saved = app.project();

  CHECK_EQ(ctx, StorageStatus::Ok, storage.save_preset(1U, saved));
  CHECK_EQ(ctx, StorageStatus::Ok, storage.validate_preset(1U));

  ProjectState restored{};
  CHECK_EQ(ctx, StorageStatus::Ok, storage.load_preset(1U, restored));
  CHECK_TRUE(ctx, projects_equal(saved, restored));
  return true;
}

bool test_storage_selects_preferred_startup_slot(CheckContext& ctx) {
  StorageMetadataV1 metadata = StorageEngine::build_default_metadata();
  uint8_t slot = 0U;

  CHECK_TRUE(ctx, !StorageEngine::preferred_startup_slot(metadata, slot));

  metadata.last_saved_slot = 4U;
  CHECK_TRUE(ctx, StorageEngine::preferred_startup_slot(metadata, slot));
  CHECK_EQ(ctx, 4U, slot);

  metadata.last_loaded_slot = 2U;
  metadata.last_saved_slot = 6U;
  CHECK_TRUE(ctx, StorageEngine::preferred_startup_slot(metadata, slot));
  CHECK_EQ(ctx, 2U, slot);
  return true;
}

bool test_fram_i2c_backend_roundtrip_and_chunking(CheckContext& ctx) {
  MockFramI2cPort port;
  FramI2cBackend backend(port);
  StorageEngine storage(backend);
  App app;

  app.seed_demo_project();
  const ProjectState saved = app.project();

  CHECK_TRUE(ctx, backend.begin());
  CHECK_TRUE(ctx, port.begin_called());
  CHECK_EQ(ctx, kDefaultFramI2cAddress, port.last_probe_address());

  CHECK_EQ(ctx, StorageStatus::Ok, storage.save_metadata(StorageEngine::build_default_metadata()));
  CHECK_EQ(ctx, StorageStatus::Ok, storage.save_preset(2U, saved));

  ProjectState restored{};
  CHECK_EQ(ctx, StorageStatus::Ok, storage.load_preset(2U, restored));
  CHECK_TRUE(ctx, projects_equal(saved, restored));
  CHECK_TRUE(ctx, port.max_write_payload() <= static_cast<uint8_t>(kFramI2cMaxTransferSize + 2U));
  CHECK_TRUE(ctx, port.max_read_payload() <= kFramI2cMaxTransferSize);
  CHECK_TRUE(ctx, port.data_write_count() > 1U);
  CHECK_TRUE(ctx, port.data_read_count() > 1U);
  return true;
}

bool test_ui_scanner_emits_mode_short_and_long(CheckContext& ctx) {
  UiScanner scanner(5U, 20U);
  UiInputSnapshot snapshot{};
  UiInputEvent event{};

  snapshot.mode_button = true;
  scanner.update(snapshot, 5U);
  scanner.update(snapshot, 5U);
  snapshot.mode_button = false;
  scanner.update(snapshot, 5U);

  CHECK_TRUE(ctx, scanner.pop_event(event));
  CHECK_EQ(ctx, UiInputEventType::ModeShortPressed, event.type);
  CHECK_TRUE(ctx, !scanner.pop_event(event));

  snapshot.mode_button = true;
  scanner.update(snapshot, 5U);
  scanner.update(snapshot, 10U);
  scanner.update(snapshot, 10U);

  CHECK_TRUE(ctx, scanner.pop_event(event));
  CHECK_EQ(ctx, UiInputEventType::ModeLongPressed, event.type);

  snapshot.mode_button = false;
  scanner.update(snapshot, 5U);
  CHECK_TRUE(ctx, !scanner.pop_event(event));
  return true;
}

bool test_oled_display_initializes_and_renders(CheckContext& ctx) {
  MockOledI2cPort port;
  OledDisplay display(port);
  DisplayFrame frame{};

  CHECK_TRUE(ctx, display.begin());
  CHECK_TRUE(ctx, port.begin_called());
  CHECK_EQ(ctx, kDefaultOledI2cAddress, port.last_device_address());
  CHECK_TRUE(ctx, port.command_packet_count() > 0U);
  CHECK_TRUE(ctx, port.data_packet_count() > 0U);
  CHECK_TRUE(ctx, port.max_packet_size() <= static_cast<uint8_t>(kOledMaxTransferSize + 1U));

  strcpy(frame.lines[0], "Dual Stop Int 120.0");
  strcpy(frame.lines[1], "A> s01 p00 l08 c01");
  strcpy(frame.lines[2], "B  s01 p00 l05 c02");
  strcpy(frame.lines[3], "R3:Prob Enc:Note");

  CHECK_TRUE(ctx, display.render(frame));
  CHECK_TRUE(ctx, port.saw_non_zero_pixels());
  return true;
}

bool test_boot_screen_renders_branding_and_version(CheckContext& ctx) {
  App app;
  UiController ui(app);
  DisplayEngine display;
  DisplayFrame frame{};

  ui.enter_boot_page();
  display.render(app, ui, frame);

  CHECK_TRUE(ctx, strstr(frame.lines[1], "DIX PAS") != nullptr);
  CHECK_TRUE(ctx, strstr(frame.lines[2], "by Dr. John") != nullptr);
  CHECK_TRUE(ctx, strstr(frame.lines[3], kFirmwareVersion) != nullptr);

  ui.leave_boot_page();
  CHECK_EQ(ctx, UiPage::Home, ui.page());
  return true;
}

bool test_boot_exit_shows_startup_overlay(CheckContext& ctx) {
  App app;
  UiController ui(app);

  ui.set_startup_message("Loaded P2");
  ui.enter_boot_page();
  ui.leave_boot_page();

  CHECK_EQ(ctx, UiPage::Home, ui.page());
  CHECK_TRUE(ctx, ui.overlay().active);
  CHECK_TRUE(ctx, strstr(ui.overlay().line2, "Loaded P2") != nullptr);
  return true;
}

bool test_external_midi_clock_advances_transport(CheckContext& ctx) {
  App app;
  app.seed_demo_project();
  app.set_clock_source(ClockSource::ExternalMidi);

  CHECK_EQ(ctx, TransportState::Stopped, app.transport_state());
  app.receive_midi_byte(0xFAU);
  CHECK_EQ(ctx, TransportState::Playing, app.transport_state());
  CHECK_EQ(ctx, 0U, app.current_tick());

  app.receive_midi_byte(0xF8U);
  CHECK_EQ(ctx, static_cast<uint32_t>(kInternalTicksPerMidiClock), app.current_tick());

  app.receive_midi_byte(0xFCU);
  CHECK_EQ(ctx, TransportState::Stopped, app.transport_state());
  return true;
}

bool test_ui_generates_and_mutates_slots_from_global_edit(CheckContext& ctx) {
  App app;
  MemoryStorageBackend backend;
  StorageEngine storage(backend);
  UiController ui(app, &storage);

  app.seed_demo_project();
  ui.reset();
  ui.press_mode_long();
  cycle_to_global_target(ui, GlobalTarget::GenerativeSlot);

  CHECK_EQ(ctx, UiPage::GlobalEdit, ui.page());
  CHECK_EQ(ctx, GlobalTarget::GenerativeSlot, ui.global_target());
  CHECK_EQ(ctx, 0U, ui.generative_slot());

  ui.press_encoder_button();
  const ProjectState applied_dorian = app.project();

  CHECK_EQ(ctx, 9U, applied_dorian.root_note);
  CHECK_EQ(ctx, 5U, applied_dorian.scale_id);
  CHECK_EQ(ctx, MachineMode::Dual, applied_dorian.machine_mode);
  CHECK_TRUE(ctx, count_active_steps(applied_dorian.track_a) > 0U);
  CHECK_TRUE(ctx, count_active_steps(applied_dorian.track_b) > 0U);

  ui.set_shift_held(true);
  ui.press_encoder_button();
  ui.set_shift_held(false);
  const ProjectState mutated_dorian = app.project();

  CHECK_EQ(ctx, applied_dorian.root_note, mutated_dorian.root_note);
  CHECK_EQ(ctx, applied_dorian.scale_id, mutated_dorian.scale_id);
  CHECK_TRUE(ctx, !projects_equal(applied_dorian, mutated_dorian));

  ui.rotate_encoder(1);
  CHECK_EQ(ctx, 1U, ui.generative_slot());
  ui.press_encoder_button();

  const ProjectState applied_chromatic = app.project();
  CHECK_EQ(ctx, 0U, applied_chromatic.root_note);
  CHECK_EQ(ctx, 11U, applied_chromatic.scale_id);
  CHECK_EQ(ctx, MachineMode::Chain20, applied_chromatic.machine_mode);
  CHECK_EQ(ctx, PlayMode::PingPong, applied_chromatic.play_mode);
  return true;
}

bool test_ui_can_switch_clock_source_from_global_edit(CheckContext& ctx) {
  App app;
  UiController ui(app);

  CHECK_EQ(ctx, ClockSource::Internal, app.clock_source());

  ui.press_mode_long();
  cycle_to_global_target(ui, GlobalTarget::ClockSource);
  CHECK_EQ(ctx, UiPage::GlobalEdit, ui.page());
  CHECK_EQ(ctx, GlobalTarget::ClockSource, ui.global_target());

  ui.rotate_encoder(1);
  CHECK_EQ(ctx, ClockSource::ExternalMidi, app.clock_source());
  CHECK_TRUE(ctx, strstr(ui.overlay().line2, "External") != nullptr);

  ui.rotate_encoder(1);
  CHECK_EQ(ctx, ClockSource::Internal, app.clock_source());
  CHECK_TRUE(ctx, strstr(ui.overlay().line2, "Internal") != nullptr);
  return true;
}

bool test_ui_can_switch_machine_mode_from_global_edit(CheckContext& ctx) {
  App app;
  UiController ui(app);

  CHECK_EQ(ctx, MachineMode::Dual, app.project().machine_mode);

  ui.press_mode_long();
  cycle_to_global_target(ui, GlobalTarget::MachineMode);
  CHECK_EQ(ctx, UiPage::GlobalEdit, ui.page());
  CHECK_EQ(ctx, GlobalTarget::MachineMode, ui.global_target());

  ui.rotate_encoder(1);
  CHECK_EQ(ctx, MachineMode::Chain20, app.project().machine_mode);
  CHECK_TRUE(ctx, strstr(ui.overlay().line2, "Chain20") != nullptr);

  ui.rotate_encoder(1);
  CHECK_EQ(ctx, MachineMode::Dual, app.project().machine_mode);
  CHECK_TRUE(ctx, strstr(ui.overlay().line2, "Dual") != nullptr);
  return true;
}

bool test_chain20_uses_single_visible_midi_channel(CheckContext& ctx) {
  App app;
  ProjectState project = app.project();
  project.track_b.midi_channel = 7U;
  app.load_project(project);
  UiController ui(app);
  DisplayEngine display;
  DisplayFrame frame{};

  ui.press_mode_long();
  cycle_to_global_target(ui, GlobalTarget::MachineMode);
  ui.rotate_encoder(1);
  CHECK_EQ(ctx, MachineMode::Chain20, app.project().machine_mode);

  cycle_to_global_target(ui, GlobalTarget::MidiChannel);
  ui.rotate_encoder(1);
  CHECK_EQ(ctx, 2U, app.project().track_a.midi_channel);
  CHECK_EQ(ctx, 7U, app.project().track_b.midi_channel);
  CHECK_TRUE(ctx, strstr(ui.overlay().line2, "Chain 2") != nullptr);

  display.render(app, ui, frame);

  ui.press_mode_long();
  display.render(app, ui, frame);
  CHECK_TRUE(ctx, strstr(frame.lines[1], "c02") != nullptr);
  CHECK_TRUE(ctx, strstr(frame.lines[2], "c02") != nullptr);
  return true;
}

bool test_shift_reset_toggles_diagnostic_mode(CheckContext& ctx) {
  App app;
  UiController ui(app);
  DisplayEngine display;
  DisplayFrame frame{};

  ui.set_hardware_status(true, false);
  ui.set_shift_held(true);
  ui.press_reset();
  CHECK_EQ(ctx, UiPage::Diagnostic, ui.page());

  ui.press_track_step(TrackId::A, 2U);
  CHECK_TRUE(ctx, strstr(ui.diagnostic_event(), "A Step 3") != nullptr);

  display.render(app, ui, frame);
  CHECK_TRUE(ctx, strstr(frame.lines[0], "Diagnostic") != nullptr);
  CHECK_TRUE(ctx, strstr(frame.lines[1], "FRAM:OK OLED:MISS") != nullptr);

  ui.press_reset();
  CHECK_EQ(ctx, UiPage::Home, ui.page());
  return true;
}

bool test_diagnostic_shows_last_midi_input_event(CheckContext& ctx) {
  App app;
  UiController ui(app);
  DisplayEngine display;
  DisplayFrame frame{};

  app.receive_midi_byte(0xFAU);
  ui.set_shift_held(true);
  ui.press_reset();

  display.render(app, ui, frame);
  CHECK_TRUE(ctx, strstr(frame.lines[3], "In:Start") != nullptr);

  app.receive_midi_byte(0xFCU);
  display.render(app, ui, frame);
  CHECK_TRUE(ctx, strstr(frame.lines[3], "In:Stop") != nullptr);
  return true;
}

bool test_shift_play_toggles_hardware_test_and_emits_outputs(CheckContext& ctx) {
  App app;
  UiController ui(app);
  DisplayEngine display;
  DisplayFrame frame{};

  ui.set_shift_held(true);
  ui.press_play();
  CHECK_EQ(ctx, UiPage::HardwareTest, ui.page());
  ui.set_shift_held(false);
  CHECK_TRUE(ctx, ui.hardware_test_running());
  CHECK_EQ(ctx, HardwareTestMode::Combined, ui.hardware_test_mode());
  CHECK_TRUE(ctx, app.gate_state(TrackId::A));
  CHECK_TRUE(ctx, !app.gate_state(TrackId::B));
  CHECK_TRUE(ctx, strstr(ui.hardware_test_status(), "A On") != nullptr);

  uint8_t byte = 0U;
  CHECK_TRUE(ctx, app.pop_midi_byte(byte));
  CHECK_EQ(ctx, 0x90U, byte);
  CHECK_TRUE(ctx, app.pop_midi_byte(byte));
  CHECK_TRUE(ctx, app.pop_midi_byte(byte));
  CHECK_TRUE(ctx, byte > 0U);

  display.render(app, ui, frame);
  CHECK_TRUE(ctx, strstr(frame.lines[0], "HW Both Run") != nullptr);
  CHECK_TRUE(ctx, strstr(frame.lines[2], "GA:Hi GB:Lo") != nullptr);

  ui.update(500U);
  CHECK_TRUE(ctx, !app.gate_state(TrackId::A));
  CHECK_TRUE(ctx, strstr(ui.hardware_test_status(), "A Off") != nullptr);
  CHECK_TRUE(ctx, app.pop_midi_byte(byte));
  CHECK_EQ(ctx, 0x80U, byte);

  ui.press_mode_short();
  CHECK_EQ(ctx, HardwareTestMode::MidiOnly, ui.hardware_test_mode());
  CHECK_TRUE(ctx, !app.gate_state(TrackId::A));
  CHECK_TRUE(ctx, !app.gate_state(TrackId::B));

  ui.press_stop();
  CHECK_TRUE(ctx, !ui.hardware_test_running());
  CHECK_TRUE(ctx, strstr(ui.hardware_test_status(), "Hold Low") != nullptr);

  ui.press_play();
  CHECK_TRUE(ctx, ui.hardware_test_running());
  CHECK_TRUE(ctx, strstr(ui.hardware_test_status(), "A On") != nullptr);

  ui.set_shift_held(true);
  ui.press_play();
  CHECK_EQ(ctx, UiPage::Home, ui.page());
  CHECK_TRUE(ctx, ui.overlay().active);
  CHECK_TRUE(ctx, strstr(ui.overlay().line2, "HW Test Off") != nullptr);
  return true;
}

using TestFunction = bool (*)(CheckContext&);

struct TestCase {
  const char* name;
  TestFunction fn;
};

}  // namespace

int main() {
  const TestCase tests[] = {
      {"scale_registry_and_quantization", test_scale_registry_and_quantization},
      {"storage_roundtrip_preserves_project", test_storage_roundtrip_preserves_project},
      {"storage_selects_preferred_startup_slot", test_storage_selects_preferred_startup_slot},
      {"fram_i2c_backend_roundtrip_and_chunking", test_fram_i2c_backend_roundtrip_and_chunking},
      {"ui_scanner_emits_mode_short_and_long", test_ui_scanner_emits_mode_short_and_long},
      {"oled_display_initializes_and_renders", test_oled_display_initializes_and_renders},
      {"boot_screen_renders_branding_and_version",
       test_boot_screen_renders_branding_and_version},
      {"boot_exit_shows_startup_overlay", test_boot_exit_shows_startup_overlay},
      {"external_midi_clock_advances_transport", test_external_midi_clock_advances_transport},
      {"ui_generates_and_mutates_slots_from_global_edit",
       test_ui_generates_and_mutates_slots_from_global_edit},
      {"ui_can_switch_clock_source_from_global_edit",
       test_ui_can_switch_clock_source_from_global_edit},
      {"ui_can_switch_machine_mode_from_global_edit",
       test_ui_can_switch_machine_mode_from_global_edit},
      {"chain20_uses_single_visible_midi_channel",
       test_chain20_uses_single_visible_midi_channel},
      {"shift_reset_toggles_diagnostic_mode", test_shift_reset_toggles_diagnostic_mode},
      {"diagnostic_shows_last_midi_input_event", test_diagnostic_shows_last_midi_input_event},
      {"shift_play_toggles_hardware_test_and_emits_outputs",
       test_shift_play_toggles_hardware_test_and_emits_outputs},
  };

  uint8_t passed = 0U;
  const uint8_t total = static_cast<uint8_t>(sizeof(tests) / sizeof(tests[0]));

  std::puts("DIX PAS native checks");
  for (uint8_t index = 0; index < total; ++index) {
    CheckContext ctx{};
    ctx.test_name = tests[index].name;
    std::printf("-> %s\n", ctx.test_name);

    const bool ok = tests[index].fn(ctx);
    if (ok && !ctx.failed) {
      ++passed;
      std::puts("  OK");
    } else {
      std::puts("  FAIL");
    }
  }

  std::printf("summary: %u/%u passed\n", static_cast<unsigned>(passed),
              static_cast<unsigned>(total));
  return passed == total ? 0 : 1;
}

#endif
