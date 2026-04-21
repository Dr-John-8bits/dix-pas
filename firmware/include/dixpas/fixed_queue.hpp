#pragma once

#include <stddef.h>
#include <stdint.h>

namespace dixpas {

template <typename T, size_t Capacity>
class FixedQueue {
 public:
  bool push(const T& value) {
    if (size_ >= Capacity) {
      overflowed_ = true;
      if (dropped_count_ != 0xFFFFFFFFU) {
        ++dropped_count_;
      }
      return false;
    }

    items_[tail_] = value;
    tail_ = (tail_ + 1U) % Capacity;
    ++size_;
    return true;
  }

  bool pop(T& value) {
    if (size_ == 0U) {
      return false;
    }

    value = items_[head_];
    head_ = (head_ + 1U) % Capacity;
    --size_;
    return true;
  }

  void clear() {
    head_ = 0U;
    tail_ = 0U;
    size_ = 0U;
    clear_overflow();
  }

  [[nodiscard]] bool empty() const { return size_ == 0U; }
  [[nodiscard]] size_t size() const { return size_; }
  [[nodiscard]] constexpr size_t capacity() const { return Capacity; }
  [[nodiscard]] bool overflowed() const { return overflowed_; }
  [[nodiscard]] uint32_t dropped_count() const { return dropped_count_; }
  void clear_overflow() {
    overflowed_ = false;
    dropped_count_ = 0U;
  }

 private:
  T items_[Capacity]{};
  size_t head_ = 0U;
  size_t tail_ = 0U;
  size_t size_ = 0U;
  bool overflowed_ = false;
  uint32_t dropped_count_ = 0U;
};

}  // namespace dixpas
