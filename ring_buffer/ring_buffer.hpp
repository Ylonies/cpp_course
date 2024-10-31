#include <cstddef>
#include <iostream>
#include <vector>

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity)
      : kCapacity(capacity), buffer_(capacity) {}

  size_t Size() const { return size_; }

  bool Empty() const { return (size_ == 0); }

  bool TryPush(int element) {
    if (size_ == kCapacity) {
      return false;
    }
    buffer_[tail_] = element;
    tail_ = IndexRecount(tail_);
    ++size_;
    return true;
  }

  bool TryPop(int* element) {
    if (Empty()) {
      return false;
    }
    *element = buffer_[head_];
    head_ = IndexRecount(head_);
    --size_;
    return true;
  }

 private:
  size_t size_ = 0;
  const size_t kCapacity;
  std::vector<int> buffer_;
  size_t head_ = 0;
  size_t tail_ = 0;

  size_t IndexRecount(size_t index) const { return (index + 1) % kCapacity; }
};
