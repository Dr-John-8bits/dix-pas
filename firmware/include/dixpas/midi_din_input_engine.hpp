#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/fixed_queue.hpp"

namespace dixpas {

enum class MidiInputEventType : uint8_t {
  Clock = 0,
  Start = 1,
  Continue = 2,
  Stop = 3,
};

struct MidiInputEvent {
  MidiInputEventType type = MidiInputEventType::Clock;
  uint8_t raw_status = 0xF8U;

  constexpr MidiInputEvent() = default;
  constexpr MidiInputEvent(MidiInputEventType type_in, uint8_t raw_status_in)
      : type(type_in), raw_status(raw_status_in) {}
};

class MidiDinInputEngine {
 public:
  static constexpr size_t kEventQueueCapacity = 64;

  void reset();
  void feed_byte(uint8_t byte);
  bool pop_event(MidiInputEvent& event);
  [[nodiscard]] bool has_overflowed() const { return event_queue_.overflowed(); }
  [[nodiscard]] uint32_t dropped_event_count() const { return event_queue_.dropped_count(); }
  void clear_overflow() { event_queue_.clear_overflow(); }

 private:
  FixedQueue<MidiInputEvent, kEventQueueCapacity> event_queue_{};

  void push_realtime_event(MidiInputEventType type, uint8_t status);
};

}  // namespace dixpas
