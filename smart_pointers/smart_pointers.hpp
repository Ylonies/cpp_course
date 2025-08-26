
template <typename T>
class SharedPtr {
  SharedPtr() = default;

  template <typename Y>
  SharedPtr(Y* ptr);

  template <typename Y>
  SharedPtr(const SharedPtr<Y>&);

  template <typename Y>
  SharedPtr(SharedPtr<Y>&&);

  template <typename Y, typename Deleter>
  SharedPtr(Y* ptr, Deleter);

  template <typename Y, typename Deleter, typename Alloc>
  SharedPtr(Y* ptr, Deleter, Alloc)
};
