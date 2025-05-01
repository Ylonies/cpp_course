#include <iostream>
#include <iterator>

template <typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  struct BaseNode {
    BaseNode* prev_node;
    BaseNode* next_node;
    BaseNode() : prev_node(this), next_node(this) {}
  };

  struct Node : public BaseNode {
    Node(const T& val) : BaseNode(), value(val) {}
    Node() = default;
    T value;
  };

 public:
  using value_type = T;
  using allocator_type = Allocator;

  using alloc_traits = std::allocator_traits<Allocator>;
  using node_alloc = typename alloc_traits::template rebind_alloc<Node>;
  using node_alloc_traits = typename alloc_traits::template rebind_traits<Node>;

  template <bool IsConst>
  class Iterator;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

  const_iterator cbegin() const;
  const_iterator cend() const;

  reverse_iterator rbegin();

  reverse_iterator rend();

  const_reverse_iterator crbegin() const;

  const_reverse_iterator crend() const;

  List() = default;

  List(size_t count, const T& value, const Allocator& alloc = Allocator());

  explicit List(size_t count, const Allocator& alloc = Allocator());

  List(const List& other);

  List(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  List& operator=(const List& other);

  T& front();
  const T& front() const;

  T& back();
  const T& back() const;

  bool empty() const;
  size_t size() const;

  void push_back(const T& value = T());

  void push_front(const T& value);

  void pop_back();

  void pop_front();

  Allocator get_allocator();

  ~List() { clear(); }

 private:
  void clear();
  BaseNode fake_node_;
  node_alloc alloc_;
  size_t size_ = 0;
};

template <typename T, typename Allocator>
template <bool IsConst>
class List<T, Allocator>::Iterator {
 public:
  using value_type = std::conditional_t<IsConst, const T, T>;
  using node_t = std::conditional_t<IsConst, const Node, Node>;
  using base_node_t = std::conditional_t<IsConst, const BaseNode, BaseNode>;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;

  Iterator() = default;
  Iterator(base_node_t* node_ptr) : iter_node_(node_ptr) {}
  Iterator(const Iterator& other) : iter_node_(other.iter_node_) {}
  explicit operator Iterator<true>() const;
  Iterator& operator=(const Iterator& iter);
  bool operator==(const Iterator& other);
  bool operator!=(const Iterator& other);
  Iterator& operator++();
  Iterator operator++(int);
  Iterator& operator--();
  Iterator<IsConst> operator--(int);
  reference operator*() const;
  pointer operator->() const;

 private:
  base_node_t* iter_node_;
};

template <typename T, typename Allocator>
template <bool IsConst>
List<T, Allocator>::Iterator<IsConst>::operator Iterator<true>() const {
  return Iterator<true>(iter_node_);
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template Iterator<IsConst>&
List<T, Allocator>::Iterator<IsConst>::operator=(const Iterator& iter) {
  if (&iter == this) {
    return *this;
  }
  iter_node_ = iter.iter_node_;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
bool List<T, Allocator>::Iterator<IsConst>::operator==(const Iterator& other) {
  return iter_node_ == other.iter_node_;
}

template <typename T, typename Allocator>
template <bool IsConst>
bool List<T, Allocator>::Iterator<IsConst>::operator!=(const Iterator& other) {
  return !(*this == other);
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template Iterator<IsConst>&
List<T, Allocator>::Iterator<IsConst>::operator++() {
  iter_node_ = iter_node_->next_node;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template Iterator<IsConst>
List<T, Allocator>::Iterator<IsConst>::operator++(int) {
  Iterator temp(*this);
  ++(*this);
  return temp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template Iterator<IsConst>&
List<T, Allocator>::Iterator<IsConst>::operator--() {
  iter_node_ = iter_node_->prev_node;
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template Iterator<IsConst>
List<T, Allocator>::Iterator<IsConst>::operator--(int) {
  Iterator temp(*this);
  --(*this);
  return temp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template Iterator<IsConst>::reference
List<T, Allocator>::Iterator<IsConst>::operator*() const {
  return static_cast<node_t*>(iter_node_)->value;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename List<T, Allocator>::template Iterator<IsConst>::pointer
List<T, Allocator>::Iterator<IsConst>::operator->() const {
  return &static_cast<node_t*>(iter_node_)->value;
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::begin() {
  return iterator(fake_node_.next_node);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::begin() const {
  return const_iterator(fake_node_.next_node);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::iterator List<T, Allocator>::end() {
  return iterator(&fake_node_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
  return const_iterator(&fake_node_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const {
  return const_iterator(fake_node_.next_node);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
  return const_iterator(&fake_node_);
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
  return std::make_reverse_iterator(iterator(&fake_node_));
}

template <typename T, typename Allocator>
typename List<T, Allocator>::reverse_iterator List<T, Allocator>::rend() {
  return std::make_reverse_iterator(iterator(fake_node_.next_node));
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator
List<T, Allocator>::crbegin() const {
  return std::make_reverse_iterator(const_iterator(&fake_node_));
}

template <typename T, typename Allocator>
typename List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend()
    const {
  return std::make_reverse_iterator(const_iterator(fake_node_.next_node));
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc)
    : alloc_(alloc) {
  try {
    for (size_t i = 0; i < count; ++i) {
      push_back(value);
    }
  } catch (...) {
    clear();
    throw;
  }
};

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc) : alloc_(alloc) {
  for (size_t i = 0; i < count; ++i) {
    Node* new_node = node_alloc_traits::allocate(alloc_, 1);
    try {
      node_alloc_traits::construct(alloc_, new_node);
    } catch (...) {
      node_alloc_traits::deallocate(alloc_, new_node, 1);
      clear();
      throw;
    }
    new_node->prev_node = fake_node_.prev_node;
    new_node->next_node = &fake_node_;
    fake_node_.prev_node->next_node = new_node;
    fake_node_.prev_node = new_node;
    ++size_;
  }
};

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : alloc_(node_alloc_traits::select_on_container_copy_construction(
          other.alloc_)) {
  try {
    for (auto it = other.cbegin(); it != other.cend(); ++it) {
      push_back(*it);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<T> init, const Allocator& alloc)
    : alloc_(alloc) {
  try {
    for (auto it = init.begin(); it != init.end(); ++it) {
      push_back(*it);
    }
  } catch (...) {
    clear();
    throw;
  }
};

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List& other) {
  if (this == &other) {
    return *this;
  }
  BaseNode new_fake_node;
  node_alloc new_alloc =
      node_alloc_traits::select_on_container_copy_construction(other.alloc_);
  size_t new_size = 0;
  Node* current = nullptr;

  try {
    for (auto it = other.cbegin(); it != other.cend(); ++it) {
      current = node_alloc_traits::allocate(new_alloc, 1);
      try {
        node_alloc_traits::construct(new_alloc, current, *it);
      } catch (...) {
        node_alloc_traits::deallocate(new_alloc, current, 1);
        throw;
      }
      current->prev_node = new_fake_node.prev_node;
      current->next_node = &new_fake_node;
      new_fake_node.prev_node->next_node = current;
      new_fake_node.prev_node = current;
      new_size++;
    }
    clear();

    if (new_size > 0) {
      fake_node_.next_node = new_fake_node.next_node;
      fake_node_.prev_node = new_fake_node.prev_node;
      fake_node_.next_node->prev_node = &fake_node_;
      fake_node_.prev_node->next_node = &fake_node_;
    } else {
      fake_node_.next_node = &fake_node_;
      fake_node_.prev_node = &fake_node_;
    }

    size_ = new_size;
    if (node_alloc_traits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
    }

  } catch (...) {
    while (new_fake_node.next_node != &new_fake_node) {
      Node* to_delete = static_cast<Node*>(new_fake_node.next_node);
      new_fake_node.next_node = to_delete->next_node;
      node_alloc_traits::destroy(new_alloc, to_delete);
      node_alloc_traits::deallocate(new_alloc, to_delete, 1);
    }
    throw;
  }
  return *this;
}

template <typename T, typename Allocator>
T& List<T, Allocator>::front() {
  return static_cast<Node*>(fake_node_.next_node)->value;
}

template <typename T, typename Allocator>
const T& List<T, Allocator>::front() const {
  return static_cast<Node*>(fake_node_.next_node)->value;
}

template <typename T, typename Allocator>
T& List<T, Allocator>::back() {
  return static_cast<Node*>(fake_node_.prev_node)->value;
}

template <typename T, typename Allocator>
const T& List<T, Allocator>::back() const {
  return static_cast<Node*>(fake_node_.prev_node)->value;
}

template <typename T, typename Allocator>
bool List<T, Allocator>::empty() const {
  return size_ == 0;
}

template <typename T, typename Allocator>
size_t List<T, Allocator>::size() const {
  return size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& value) {
  Node* new_node = node_alloc_traits::allocate(alloc_, 1);
  try {
    node_alloc_traits::construct(alloc_, new_node, value);
  } catch (...) {
    node_alloc_traits::deallocate(alloc_, new_node, 1);
    throw;
  }
  new_node->prev_node = fake_node_.prev_node;
  new_node->next_node = &fake_node_;
  fake_node_.prev_node->next_node = new_node;
  fake_node_.prev_node = new_node;

  ++size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value) {
  Node* new_node = nullptr;
  try {
    new_node = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, new_node, value);

    new_node->next_node = fake_node_.next_node;
    new_node->prev_node = &fake_node_;
    fake_node_.next_node->prev_node = new_node;
    fake_node_.next_node = new_node;

    ++size_;
  } catch (...) {
    if (new_node) {
      node_alloc_traits::deallocate(alloc_, new_node, 1);
    }
    throw;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  BaseNode* current = fake_node_.prev_node;
  fake_node_.prev_node = current->prev_node;
  current->prev_node->next_node = &fake_node_;
  node_alloc_traits::destroy(alloc_, static_cast<Node*>(current));
  node_alloc_traits::deallocate(alloc_, static_cast<Node*>(current), 1);
  --size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  BaseNode* current = fake_node_.next_node;
  fake_node_.next_node = current->next_node;
  current->next_node->prev_node = &fake_node_;
  node_alloc_traits::destroy(alloc_, static_cast<Node*>(current));
  node_alloc_traits::deallocate(alloc_, static_cast<Node*>(current), 1);
  --size_;
}

template <typename T, typename Allocator>
Allocator List<T, Allocator>::get_allocator() {
  return alloc_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::clear() {
  BaseNode* current = fake_node_.next_node;
  while (current != &fake_node_) {
    BaseNode* next = current->next_node;
    node_alloc_traits::destroy(alloc_, static_cast<Node*>(current));
    node_alloc_traits::deallocate(alloc_, static_cast<Node*>(current), 1);
    current = next;
  }
  size_ = 0;
}