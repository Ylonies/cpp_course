#include <math.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 public:
  template <bool IsConst>
  class Iterator;

  using value_type = T;
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<Allocator>;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(start_, map_.data()); }

  const_iterator cbegin() const { return const_iterator(start_, map_.data()); }

  iterator end() { return iterator(end_, map_.data()); }

  const_iterator cend() const { return const_iterator(end_, map_.data()); }

  reverse_iterator rbegin() {
    return std::make_reverse_iterator(iterator(end_, map_.data()));
  }

  const_iterator crbegin() const {
    return std::make_reverse_iterator(const_iterator(end_, map_.data()));
  }

  reverse_iterator rend() {
    return std::make_reverse_iterator(iterator(start_, map_.data()));
  }

  const_iterator crend() const {
    return std::make_reverse_iterator(const_iterator(start_, map_.data()));
  }

  Deque() = default;

  ~Deque() { delete_map(map_); }

  Deque(const Allocator& alloc);
  Deque(const Deque& other);
  Deque(size_t count, const Allocator& alloc = Allocator());
  Deque(size_t count, const T& value, const Allocator& alloc = Allocator());
  Deque& operator=(const Deque& other);
  Deque& operator=(Deque&& other);
  Deque(Deque&& other);
  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  size_t size() const;
  bool empty() const;
  T& operator[](size_t index);
  const T& operator[](size_t index) const;

  T& at(size_t index);
  const T& at(size_t index) const;

  void push_back(const T& elem);
  void push_front(const T& elem);
  void pop_back();
  void pop_front();

  template <typename... Args>
  void emplace_back(Args&&... args);

  template <typename... Args>
  void emplace_front(Args&&... args);

  void insert(iterator iterator, const T& value);
  void erase(iterator iterator);

 private:
  size_t capacity() const;
  void delete_map(std::vector<T*>& map);

  void bucket_reserve(size_t bucket_index);
  void reserve(size_t new_bucket_capacity);

  size_t get_initial_start(size_t bucket_number);
  size_t count_bucket_capacity(size_t count);

  allocator_type alloc_;
  std::vector<T*> map_;
  size_t start_ = 0;
  size_t end_ = 0;
  static const size_t kBucketSize = 16;
  const size_t kMinBucketNumber = 4;
};

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::Iterator {
 public:
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  using data_type = std::conditional_t<IsConst, T* const*, T**>;
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  size_t k_bucket_size = Deque::kBucketSize;

  Iterator(size_t index, data_type data) : index_(index), data_(data) {}

  Iterator(const Iterator& other) : data_(other.data_), index_(other.index_) {}

  Iterator& operator=(const Iterator& iter) {
    if (iter == *this) {
      return *this;
    }
    data_ = iter.data_;
    index_ = iter.index_;
    return *this;
  }

  Iterator<IsConst>& operator++() {
    ++index_;
    return *this;
  }

  Iterator<IsConst> operator++(int) {
    Iterator<IsConst> temp(index_, data_);
    ++index_;
    return temp;
  }

  Iterator<IsConst>& operator--() {
    --index_;
    return *this;
  }

  Iterator<IsConst> operator--(int) {
    Iterator<IsConst> temp(index_, data_);
    --index_;
    return temp;
  }

  Iterator<IsConst> operator+(size_t number) const {
    return Iterator<IsConst>(index_ + number, data_);
  }

  Iterator<IsConst> operator-(size_t number) const {
    return Iterator<IsConst>(index_ - number, data_);
  }

  Iterator<IsConst>& operator+=(size_t number) {
    index_ += number;
    return *this;
  }

  Iterator<IsConst>& operator-=(size_t number) {
    index_ -= number;
    return *this;
  }

  bool operator==(const Iterator<IsConst>& other) const {
    return data_ == other.data_ && index_ == other.index_;
  }

  std::strong_ordering operator<=>(const Iterator& other) const {
    if (data_ != other.data_) {
      return std::strong_ordering::less;
    }
    return index_ <=> other.index_;
  }

  difference_type operator-(const Iterator& other) const {
    return index_ - other.index_;
  }

  reference operator*() { return *get_ptr(index_); }

  reference operator*() const { return *get_ptr(index_); }

  pointer operator->() { return get_ptr(index_); }

  pointer operator->() const { return get_ptr(index_); }

  operator Iterator<true>() const { return Iterator<true>{index_, data_}; }

 private:
  pointer get_ptr(int index) {
    return *(data_ + index / kBucketSize) + (index % kBucketSize);
  }

  data_type data_;
  size_t index_;
};

template <typename T, typename Allocator>
size_t Deque<T, Allocator>::capacity() const {
  return kBucketSize * map_.size();
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::bucket_reserve(size_t bucket_index) {
  if (map_[bucket_index] != nullptr) {
    return;
  }
  // delete[] map_[bucket_index];
  // T* my_bucket = reinterpret_cast<T*>(new int8_t[kBucketSize * sizeof(T)]);
  T* my_bucket = alloc_traits::allocate(alloc_, kBucketSize);
  map_[bucket_index] = my_bucket;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::reserve(size_t new_bucket_capacity) {
  if (map_.size() >= new_bucket_capacity) {
    return;
  }
  auto new_map = std::vector<T*>(new_bucket_capacity);
  int start_bucket = start_ / kBucketSize;
  int end_bucket = (end_ - 1) / kBucketSize;
  int bucket_size = end_bucket - start_bucket + 1;
  int spare_size = (new_bucket_capacity - bucket_size) / 2;
  for (int i = 0; i < bucket_size; ++i) {
    new_map[spare_size + i] = map_[start_bucket + i];
  }
  map_ = new_map;
  size_t size = end_ - start_;
  start_ = spare_size * kBucketSize;
  end_ = start_ + size;
}  // reserve-realization

template <typename T, typename Allocator>
void Deque<T, Allocator>::delete_map(std::vector<T*>& map) {
  for (T* bucket : map_) {
    for (size_t i = 0; i < kBucketSize; ++i) {
      alloc_traits::destroy(alloc_, &bucket[i]);
    }
    alloc_traits::deallocate(alloc_, bucket, kBucketSize);
  }
}

template <typename T, typename Allocator>
size_t Deque<T, Allocator>::get_initial_start(size_t bucket_number) {
  return bucket_number / 4 * kBucketSize;
}

template <typename T, typename Allocator>
size_t Deque<T, Allocator>::count_bucket_capacity(size_t count) {
  size_t bucket_number = (count + kBucketSize - 1) / kBucketSize;
  bucket_number =
      static_cast<size_t>(std::pow(2, std::ceil(std::log2(bucket_number))));
  bucket_number *= 2;
  return std::max(bucket_number, kMinBucketNumber);
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Allocator& alloc) : alloc_(alloc) {}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& other)
    : start_(other.start_),
      end_(other.end_),
      alloc_(
          alloc_traits::select_on_container_copy_construction(other.alloc_)) {
  for (size_t i = 0; i < other.map_.size(); ++i) {
    T* my_bucket = alloc_traits::allocate(alloc_, kBucketSize);
    map_.push_back(my_bucket);
  }

  try {
    for (auto elem = begin(); elem != end(); ++elem) {
      size_t index = elem - begin();
      alloc_traits::construct(alloc_, &(*elem), other[index]);
      // *elem = other[index];
    }
  } catch (...) {
    delete_map(map_);
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const T& value, const Allocator& alloc)
    : alloc_(alloc) {
  try {
    size_t bucket_capacity = count_bucket_capacity(count);
    map_.resize(bucket_capacity);
    start_ = get_initial_start(bucket_capacity);
    end_ = start_ + count;
    for (size_t i = start_ / kBucketSize; i <= (end_ - 1) / kBucketSize; ++i) {
      T* my_bucket = alloc_traits::allocate(alloc_, kBucketSize);
      map_[i] = my_bucket;
    }
    for (auto iter = begin(); iter < end(); ++iter) {
      alloc_traits::construct(alloc_, &(*iter), value);
    }
  } catch (...) {
    delete_map(map_);
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& alloc)
    : Deque(count, T(), alloc){};

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(const Deque& other) {
  if (this == &other) {
    return *this;
  }

  Deque<T, Allocator> temp;
  if (alloc_traits::propagate_on_container_copy_assignment::value) {
    temp.alloc_ = other.alloc_;
  } else {
    temp.alloc_ = alloc_;
  }
  for (size_t i = 0; i < other.map_.size(); ++i) {
    T* my_bucket = alloc_traits::allocate(temp.alloc_, kBucketSize);
    temp.map_.push_back(my_bucket);
  }
  try {
    for (auto elem = other.begin(); elem != other.end(); ++elem) {
      size_t index = elem - other.begin();
      alloc_traits::construct(temp.alloc_, &temp[index], *elem);
    }
  } catch (...) {
    temp.delete_map();
    throw;
  }
  std::swap(map_, temp.map_);
  std::swap(start_, temp.start_);
  std::swap(end_, temp.end_);
  std::swap(alloc_, temp.alloc_);
  return *this;
}  // operator =
template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(Deque&& other) {
  if (this == &other) {
    return *this;
  }
  if constexpr (alloc_traits::propagate_on_container_move_assignment::value) {
    alloc_ = std::move(other.alloc_);
    map_ = std::move(other.map_);
  } else {
    if (alloc_ == other.alloc_) {
      map_ = std::move(other.map_);
    } else {
      return operator=(other);
    }
  }
  start_ = other.start_;
  end_ = other.end_;
  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other) {
  if (alloc_traits::propagate_on_container_move_assignment::value) {
    alloc_ = std::move(other.alloc_);
  }
  map_ = std::move(other.map_);
  start_ = other.start_;
  end_ = other.end_;
}  // move-copy

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& alloc)
    : alloc_(alloc) {
  try {
    size_t bucket_capacity = count_bucket_capacity(init.size());
    map_.resize(bucket_capacity);
    start_ = get_initial_start(bucket_capacity);
    end_ = start_ + init.size();
    for (size_t i = start_ / kBucketSize; i <= (end_ - 1) / kBucketSize; ++i) {
      T* my_bucket = alloc_traits::allocate(alloc_, kBucketSize);
      map_[i] = my_bucket;
    }
    auto current_it = begin();
    for (const T& item : init) {
      alloc_traits::construct(alloc_, &*current_it, item);
      ++current_it;
    }
  } catch (...) {
    delete_map(map_);
    throw;
  }
}

template <typename T, typename Allocator>
size_t Deque<T, Allocator>::size() const {
  return end_ - start_;
}

template <typename T, typename Allocator>
bool Deque<T, Allocator>::empty() const {
  return (start_ == end_);
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::operator[](size_t index) {
  return *(begin() + index);
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::operator[](size_t index) const {
  return *(cbegin() + index);
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::at(size_t index) {
  if (index >= size()) {
    throw std::out_of_range("Index out of range");
  }

  return *(begin() + index);
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t index) const {
  if (index >= size()) {
    throw std::out_of_range("Index out of range");
  }
  return *(cbegin() + index);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& elem) {
  if (end_ >= capacity()) {
    reserve(std::max(kMinBucketNumber, map_.size() * 2));
  }
  if (end_ % kBucketSize == 0) {
    bucket_reserve((end_ + 1) / kBucketSize);
  }
  alloc_traits::construct(alloc_, end(), elem);
  // new (std::addressof(*end())) T(elem);
  ++end_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& elem) {
  if (start_ == 0) {
    reserve(std::max(kMinBucketNumber, map_.size() * 2));
  }
  if (start_ % kBucketSize == 0) {
    bucket_reserve((start_ - 1) / kBucketSize);
  }
  --start_;
  alloc_traits::construct(alloc_, begin(), elem);
  // new (std::addressof(*begin())) T(elem);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
  alloc_traits::destroy(alloc_, &*end());
  if (end_ % kBucketSize == 1) {
    alloc_traits::deallocate(alloc_, end_ / kBucketSize, kBucketSize);
    // delete[] map_[end_ / kBucketSize];
    map_[end_ / kBucketSize] = nullptr;
  }
  --end_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  alloc_traits::destroy(alloc_, &*begin());
  if (start_ % kBucketSize == kBucketSize - 1) {
    alloc_traits::deallocate(alloc_, end_ / kBucketSize, kBucketSize);
    // delete[] map_[start_ / kBucketSize];
    map_[start_ / kBucketSize] = nullptr;
  }
  ++start_;
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  if (end_ >= capacity()) {
    reserve(std::max(kMinBucketNumber, map_.size() * 2));
  }

  if (end_ % kBucketSize == 0) {
    bucket_reserve((end_ + 1) / kBucketSize);
  }

  alloc_traits::construct(alloc_, std::addressof(*end()),
                          std::forward<Args>(args)...);

  ++end_;
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  if (end_ >= capacity()) {
    reserve(std::max(kMinBucketNumber, map_.size() * 2));
  }

  if (end_ % kBucketSize == 0) {
    bucket_reserve((end_ + 1) / kBucketSize);
  }

  alloc_traits::construct(alloc_, std::addressof(*end()),
                          std::forward<Args>(args)...);

  ++end_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::insert(iterator insert_iterator, const T& value) {
  auto temp_value = value;
  for (auto iter = insert_iterator; iter != end(); ++iter) {
    std::swap(temp_value, *iter);
  }
  push_back(temp_value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::erase(iterator erase_iterator) {
  erase_iterator->~T();
  for (auto iter = erase_iterator; iter != end() - 1; ++iter) {
    *iter = *(iter + 1);
  }
  --end_;
}

int main() {
  Deque deque({1, 2, 3, 4});
  // std::cout << deque[2];
  // std::cout << deque.size();
  deque.emplace_back(1);
  std::cout << deque.size() << deque[4];
}