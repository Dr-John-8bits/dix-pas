#include "dixpas/midi_din_engine.hpp"

namespace dixpas {

void MidiDinEngine::reset() {
  byte_queue_.clear();
}

void MidiDinEngine::send_note_on(uint8_t midi_channel, uint8_t note, uint8_t velocity) {
  const uint8_t channel = static_cast<uint8_t>(clamp_midi_channel(midi_channel) - 1U);
  push_status_and_two_data(static_cast<uint8_t>(0x90U | channel), note, velocity);
}

void MidiDinEngine::send_note_off(uint8_t midi_channel, uint8_t note, uint8_t velocity) {
  const uint8_t channel = static_cast<uint8_t>(clamp_midi_channel(midi_channel) - 1U);
  push_status_and_two_data(static_cast<uint8_t>(0x80U | channel), note, velocity);
}

void MidiDinEngine::send_clock() {
  push_realtime(0xF8U);
}

void MidiDinEngine::send_start() {
  push_realtime(0xFAU);
}

void MidiDinEngine::send_continue() {
  push_realtime(0xFBU);
}

void MidiDinEngine::send_stop() {
  push_realtime(0xFCU);
}

bool MidiDinEngine::pop_byte(uint8_t& value) {
  return byte_queue_.pop(value);
}

void MidiDinEngine::push_status_and_two_data(uint8_t status, uint8_t data1, uint8_t data2) {
  byte_queue_.push(status);
  byte_queue_.push(static_cast<uint8_t>(data1 & 0x7FU));
  byte_queue_.push(static_cast<uint8_t>(data2 & 0x7FU));
}

void MidiDinEngine::push_realtime(uint8_t status) {
  byte_queue_.push(status);
}

}  // namespace dixpas
