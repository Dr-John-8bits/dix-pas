#pragma once

#include <stddef.h>

namespace dixpas {

template <typename T, size_t Capacity>
class FixedQueue {
 public:
  bool push(const T& value) {
    if (size_ >= Capacity) {
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
  }

  [[nodiscard]] bool empty() const { return size_ == 0U; }
  [[nodiscard]] size_t size() const { return size_; }
  [[nodiscard]] constexpr size_t capacity() const { return Capacity; }

 private:
  T items_[Capacity]{};
  size_t head_ = 0U;
  size_t tail_ = 0U;
  size_t size_ = 0U;
};

}  // namespace dixpas
