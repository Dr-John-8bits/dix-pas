#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/fixed_queue.hpp"
#include "dixpas/types.hpp"

namespace dixpas {

class MidiDinEngine {
 public:
  static constexpr size_t kByteQueueCapacity = 256;

  void reset();

  void send_note_on(uint8_t midi_channel, uint8_t note, uint8_t velocity);
  void send_note_off(uint8_t midi_channel, uint8_t note, uint8_t velocity = 0);
  void send_clock();
  void send_start();
  void send_continue();
  void send_stop();

  bool pop_byte(uint8_t& value);
  [[nodiscard]] bool has_overflowed() const { return byte_queue_.overflowed(); }
  [[nodiscard]] uint32_t dropped_byte_count() const { return byte_queue_.dropped_count(); }
  void clear_overflow() { byte_queue_.clear_overflow(); }

 private:
  FixedQueue<uint8_t, kByteQueueCapacity> byte_queue_{};

  void push_status_and_two_data(uint8_t status, uint8_t data1, uint8_t data2);
  void push_realtime(uint8_t status);
};

}  // namespace dixpas
