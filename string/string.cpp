#include "string.hpp"

#include <cstring>

void string_funcs::StrCopy(char* new_data, const char* data, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    new_data[i] = data[i];
  }
  new_data[size] = '\0';
}
template <typename T>
void string_funcs::Swap(T& first, T& second) {
  T temp = first;
  first = second;
  second = temp;
}

String::String(size_t size, const char& character)
    : size_(size), capacity_(size_), data_(new char[size + 1]) {
  for (size_t i = 0; i < size_; ++i) {
    data_[i] = character;
  }
  data_[size] = kNull;
}

String::String(const char* str) : size_(std::strlen(str)), capacity_(size_) {
  data_ = new char[size_ + 1];
  string_funcs::StrCopy(data_, str, size_);
}

String::String(const String& other)
    : size_(other.size_),
      capacity_(other.capacity_),
      data_(new char[other.capacity_ + 1]) {
  if (other.data_ != nullptr) {
    string_funcs::StrCopy(data_, other.data_, size_);
  }
}

String& String::operator=(const String& other) {
  if (this == &other) {
    return *this;
  }
  delete[] data_;
  data_ = new char[other.capacity_ + 1];
  capacity_ = other.capacity_;
  size_ = other.size_;
  string_funcs::StrCopy(data_, other.data_, size_);
  return *this;
}

void String::Reserve(size_t new_capacity) {
  if (new_capacity <= capacity_) {
    return;
  }
  char* new_data = new char[new_capacity + 1];
  string_funcs::StrCopy(new_data, data_, size_);
  delete[] data_;
  data_ = new_data;
  capacity_ = new_capacity;
}

void String::Clear() {
  size_ = 0;
  if (data_ != nullptr) {
    data_[0] = '\0';
  }
}

void String::PushBack(const char& character) {
  if (capacity_ <= size_) {
    Reserve(2 * size_ + 1);
  }
  data_[size_++] = character;
  data_[size_] = kNull;
}

void String::PopBack() {
  if (size_ > 0) {
    --size_;
    data_[size_] = '\n';
  }
}

void String::Resize(size_t new_size, char character) {
  if (new_size >= capacity_) {
    Reserve(new_size);  // change
  }
  for (size_t i = size_; i < new_size; ++i) {
    data_[i] = character;
  }
  size_ = new_size;
  data_[size_] = kNull;
}

void String::Resize(size_t new_size) { Resize(new_size, '\0'); }

void String::ShrinkToFit() {
  if (capacity_ == size_) {
    return;
  }
  char* new_data = new char[size_ + 1];
  string_funcs::StrCopy(new_data, data_, size_);
  new_data[size_] = kNull;
  delete[] data_;
  data_ = new_data;
  capacity_ = size_;
}

void String::Swap(String& other) {
  string_funcs::Swap(data_, other.data_);
  string_funcs::Swap(size_, other.size_);
  string_funcs::Swap(capacity_, other.capacity_);
}

char& String::operator[](size_t index) { return data_[index]; }

const char& String::operator[](size_t index) const { return data_[index]; }

char& String::Front() { return data_[0]; }

const char& String::Front() const { return data_[0]; };

char& String::Back() { return data_[size_ - 1]; };

const char& String::Back() const { return data_[size_ - 1]; };

bool String::Empty() const { return (size_ == 0); }

size_t String::Size() const { return size_; };

size_t String::Capacity() const { return capacity_; };

const char* String::Data() const { return data_; }

char* String::Data() { return data_; }

bool operator==(const String& first, const String& second) {
  if (first.Size() != second.Size()) {
    return false;
  }
  for (size_t i = 0; i < first.Size(); ++i) {
    if (first[i] != second[i]) {
      return false;
    }
  }
  return true;
};

bool operator!=(const String& first, const String& second) {
  return !(first == second);
};

bool operator<(const String& first, const String& second) {
  size_t index = 0;
  while (index < first.Size() && index < second.Size()) {
    if (first[index] > second[index]) {
      return false;
    }
    ++index;
  }
  return first[index] <= second[index];
};

bool operator>(const String& first, const String& second) {
  return second < first;
};

bool operator<=(const String& first, const String& second) {
  return !(first > second);
};

bool operator>=(const String& first, const String& second) {
  return !(first < second);
};

String& String::operator+=(const String& other) {
  Reserve(size_ + other.Size());
  string_funcs::StrCopy(data_ + size_, other.Data(), other.Size());
  size_ += other.Size();
  data_[size_] = '\0';
  return *this;
}

String operator+(const String& first, const String& second) {
  String new_string = first;
  new_string += second;
  return new_string;
}

std::ostream& operator<<(std::ostream& os, const String& string) {
  if (string.Data() != nullptr) {
    os << string.Data();
  } else {
    os << "";
  }
  return os;
}

std::istream& operator>>(std::istream& is, String& string) {
  char symb;
  string.Clear();
  std::istream::sentry sentry(is);
  if (!sentry) {
    return is;
  }
  while (is.get(symb) && isspace(symb) == 0) {
    string.PushBack(symb);
  }
  return is;
}

String String::operator*=(const size_t& number) {
  if (number == 0) {
    Clear();
    return *this;
  }
  Reserve(size_ * number);
  for (size_t i = 0; i < number; ++i) {
    string_funcs::StrCopy(data_ + (i * size_), data_, size_);
  }
  size_ *= number;
  data_[size_] = kNull;
  return *this;
}

String operator*(const String& string, const size_t& number) {
  String new_string(string);
  new_string *= number;
  return new_string;
}

String String::Slice(size_t left, size_t right) const {
  String new_string;
  if (right <= left || right > size_) {
    return new_string;
  }
  new_string.size_ = (right - left);
  new_string.data_ = new char[new_string.size_ + 1];
  new_string.capacity_ = new_string.size_;
  string_funcs::StrCopy(new_string.data_, data_ + left, new_string.size_);
  new_string.data_[new_string.size_] = kNull;
  return new_string;
}

std::vector<String> String::Split(const String& delim) const {
  std::vector<String> string_vect;
  String current;
  size_t delim_size = delim.Size();
  size_t index = 0;
  if (delim.Size() == 0 || size_ == 0) {
    string_vect.push_back(*this);
    return string_vect;
  }
  while (index < size_) {
    String now = Slice(index, index + delim_size);
    if (now == delim) {
      string_vect.push_back(current);
      current.Clear();
      index += delim_size;
    } else {
      current.PushBack(data_[index]);
      ++index;
    }
  }
  string_vect.push_back(current);
  return string_vect;
}

String String::Join(const std::vector<String>& strings) const {
  String new_string = "";
  if (strings.empty()) {
    return new_string;
  }
  for (size_t i = 0; i < strings.size() - 1; ++i) {
    new_string += strings[i];
    new_string += *this;
  }
  new_string += strings[strings.size() - 1];
  return new_string;
}