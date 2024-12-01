#ifndef STRING_H  // include guard
#define STRING_H

#include <iostream>
#include <vector>

namespace string_funcs {
void StrCopy(char* new_data, const char* data, size_t size);

template <typename T>
void Swap(T& first, T& second);
}  // namespace string_funcs

class String {
 public:
  String() = default;
  String(size_t size, const char& character);
  String(const char* str);
  String(const String& other);
  String& operator=(const String& other);
  ~String() { delete[] data_; }

  String Slice(size_t left, size_t right) const;

  void Reserve(size_t new_capacity);

  void Clear();
  void PushBack(const char& character);
  void PopBack();

  void Resize(size_t new_size);
  void Resize(size_t new_size, char character);
  void ShrinkToFit();

  void Swap(String& other);
  char& operator[](size_t index);
  const char& operator[](size_t index) const;

  char& Front();
  const char& Front() const;

  char& Back();
  const char& Back() const;

  bool Empty() const;

  size_t Size() const;

  size_t Capacity() const;

  char* Data();
  const char* Data() const;

  String& operator+=(const String& other);

  String operator*=(const size_t& number);

  std::vector<String> Split(const String& delim = " ") const;
  String Join(const std::vector<String>& strings) const;

 private:
  size_t size_ = 0;
  size_t capacity_ = 0;
  char* data_ = nullptr;

  static const char kNull = '\0';
};

bool operator==(const String& first, const String& second);
bool operator!=(const String& first, const String& second);
bool operator<(const String& first, const String& second);
bool operator>(const String& first, const String& second);
bool operator<=(const String& first, const String& second);
bool operator>=(const String& first, const String& second);
String operator+(const String& first, const String& second);
String operator*(const String& string, const size_t& number);

std::ostream& operator<<(std::ostream& os, const String& string);
std::istream& operator>>(std::istream& is, String& string);
#endif