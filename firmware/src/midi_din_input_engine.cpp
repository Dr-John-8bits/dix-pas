#include "dixpas/midi_din_input_engine.hpp"

namespace dixpas {

void MidiDinInputEngine::reset() {
  event_queue_.clear();
}

void MidiDinInputEngine::feed_byte(uint8_t byte) {
  switch (byte) {
    case 0xF8U:
      push_realtime_event(MidiInputEventType::Clock, byte);
      break;
    case 0xFAU:
      push_realtime_event(MidiInputEventType::Start, byte);
      break;
    case 0xFBU:
      push_realtime_event(MidiInputEventType::Continue, byte);
      break;
    case 0xFCU:
      push_realtime_event(MidiInputEventType::Stop, byte);
      break;
    default:
      break;
  }
}

bool MidiDinInputEngine::pop_event(MidiInputEvent& event) {
  return event_queue_.pop(event);
}

void MidiDinInputEngine::push_realtime_event(MidiInputEventType type, uint8_t status) {
  event_queue_.push(MidiInputEvent(type, status));
}

}  // namespace dixpas
