#include <math.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

template <typename T>
class Deque {
 public:
  template <bool IsConst>
  class Iterator;

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

  Deque(const Deque& other);
  Deque(size_t count);
  Deque(size_t count, const T& value);
  Deque& operator=(const Deque& other);

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

  void insert(iterator iterator, const T& value);
  void erase(iterator iterator);

 private:
  size_t capacity() const;
  void delete_map(std::vector<T*>& map);

  void bucket_reserve(size_t bucket_index);
  void reserve(size_t new_bucket_capacity);

  size_t get_initial_start(size_t bucket_number);
  size_t count_bucket_capacity(size_t count);
  std::vector<T*> map_;
  size_t start_ = 0;
  size_t end_ = 0;
  static const size_t kBucketSize = 16;
  const size_t kMinBucketNumber = 4;
};

template <typename T>
template <bool IsConst>
class Deque<T>::Iterator {
 public:
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  using data_type = std::conditional_t<IsConst, T* const*, T**>;
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  size_t k_bucket_size = Deque<T>::kBucketSize;

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

template <typename T>
size_t Deque<T>::capacity() const {
  return kBucketSize * map_.size();
}

template <typename T>
void Deque<T>::bucket_reserve(size_t bucket_index) {
  if (map_[bucket_index] != nullptr) {
    return;
  }
  // delete[] map_[bucket_index];
  T* my_bucket = reinterpret_cast<T*>(new int8_t[kBucketSize * sizeof(T)]);
  map_[bucket_index] = my_bucket;
}

template <typename T>
void Deque<T>::reserve(size_t new_bucket_capacity) {
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

template <typename T>
void Deque<T>::delete_map(std::vector<T*>& map) {
  for (auto iter = begin(); iter != end(); ++iter) {
    iter->~T();
  }
  for (auto bucket : map) {
    delete[] reinterpret_cast<int8_t*>(bucket);
  }
}

template <typename T>
size_t Deque<T>::get_initial_start(size_t bucket_number) {
  return bucket_number / 4 * kBucketSize;
}

template <typename T>
size_t Deque<T>::count_bucket_capacity(size_t count) {
  size_t bucket_number = (count + kBucketSize - 1) / kBucketSize;
  bucket_number =
      static_cast<size_t>(std::pow(2, std::ceil(std::log2(bucket_number))));
  bucket_number *= 2;
  return std::max(bucket_number, kMinBucketNumber);
}

template <typename T>
Deque<T>::Deque(const Deque& other) : start_(other.start_), end_(other.end_) {
  for (size_t i = 0; i < other.map_.size(); ++i) {
    T* my_bucket = reinterpret_cast<T*>(new int8_t[kBucketSize * sizeof(T)]);
    map_.push_back(my_bucket);
  }

  for (auto elem = begin(); elem != end(); ++elem) {
    size_t index = elem - begin();
    *elem = other[index];
  }
}

template <typename T>
Deque<T>::Deque(size_t count, const T& value) {
  try {
    size_t bucket_capacity = count_bucket_capacity(count);
    map_.resize(bucket_capacity);
    start_ = get_initial_start(bucket_capacity);
    end_ = start_ + count;
    for (size_t i = start_ / kBucketSize; i <= (end_ - 1) / kBucketSize; ++i) {
      T* my_bucket = reinterpret_cast<T*>(new int8_t[kBucketSize * sizeof(T)]);
      map_[i] = my_bucket;
    }
    for (auto iter = begin(); iter < end(); ++iter) {
      new (std::addressof(*iter)) T(value);
    }
  } catch (...) {
    delete_map(map_);
    throw;
  }
}

template <typename T>
Deque<T>::Deque(size_t count) : Deque(count, T()){};

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
  Deque<T> temp(other);
  std::swap(map_, temp.map_);
  std::swap(start_, temp.start_);
  std::swap(end_, temp.end_);
  return *this;
}  // copy operator

template <typename T>
size_t Deque<T>::size() const {
  return end_ - start_;
}

template <typename T>
bool Deque<T>::empty() const {
  return (start_ == end_);
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
  return *(begin() + index);
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
  return *(cbegin() + index);
}

template <typename T>
T& Deque<T>::at(size_t index) {
  if (index >= size()) {
    throw std::out_of_range("Index out of range");
  }

  return *(begin() + index);
}

template <typename T>
const T& Deque<T>::at(size_t index) const {
  if (index >= size()) {
    throw std::out_of_range("Index out of range");
  }
  return *(cbegin() + index);
}

template <typename T>
void Deque<T>::push_back(const T& elem) {
  if (end_ >= capacity()) {
    reserve(std::max(kMinBucketNumber, map_.size() * 2));
  }
  if (end_ % kBucketSize == 0) {
    bucket_reserve((end_ + 1) / kBucketSize);
  }
  new (std::addressof(*end())) T(elem);
  ++end_;
}

template <typename T>
void Deque<T>::push_front(const T& elem) {
  if (start_ == 0) {
    reserve(std::max(kMinBucketNumber, map_.size() * 2));
  }
  if (start_ % kBucketSize == 0) {
    bucket_reserve((start_ - 1) / kBucketSize);
  }
  --start_;
  new (std::addressof(*begin())) T(elem);
}

template <typename T>
void Deque<T>::pop_back() {
  if (end_ % kBucketSize == 1) {
    delete[] map_[end_ / kBucketSize];
    map_[end_ / kBucketSize] = nullptr;
  }
  --end_;
}

template <typename T>
void Deque<T>::pop_front() {
  if (start_ % kBucketSize == kBucketSize - 1) {
    delete[] map_[start_ / kBucketSize];
    map_[start_ / kBucketSize] = nullptr;
  }
  ++start_;
}

template <typename T>
void Deque<T>::insert(iterator insert_iterator, const T& value) {
  auto temp_value = value;
  for (auto iter = insert_iterator; iter != end(); ++iter) {
    std::swap(temp_value, *iter);
  }
  push_back(temp_value);
}

template <typename T>
void Deque<T>::erase(iterator erase_iterator) {
  erase_iterator->~T();
  for (auto iter = erase_iterator; iter != end() - 1; ++iter) {
    *iter = *(iter + 1);
  }
  --end_;
}