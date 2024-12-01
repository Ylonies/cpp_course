#include <iostream>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix();
  Matrix(const std::vector<std::vector<T>>& vect);
  Matrix(const T& elem);
  Matrix(const Matrix<N, M, T>& matrix);
  Matrix<N, M, T>& operator*=(const T& elem);

  Matrix<N, M, T>& operator+=(const Matrix<N, M, T>& second);

  Matrix<N, M, T>& operator-=(const Matrix<N, M, T>& second);

  Matrix<M, N, T> Transposed();

  T Trace() const;

  const T& operator()(size_t index_i, size_t index_j) const;
  T& operator()(size_t index_i, size_t index_j);

  std::vector<std::vector<T>>& Data();

 private:
  std::vector<std::vector<T>> data_;
};

// реализация

template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix() : data_(N, std::vector<T>(M, T())){};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix(const std::vector<std::vector<T>>& vect)
    : data_(vect){};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix(const T& elem) : data_(N, std::vector<T>(M, elem)) {}

template <size_t N, size_t M, typename T>
Matrix<N, M, T>::Matrix(const Matrix<N, M, T>& matrix) : data_(matrix.data_){};

namespace matrix_funcs {
template <typename T, size_t N>
T Trace(Matrix<N, N, T> matrix) {
  T sum = T();
  for (size_t i = 0; i < N; ++i) {
    sum += matrix(i, i);
  }
  return sum;
}
}  // namespace matrix_funcs

template <size_t N, size_t M, typename T>
T Matrix<N, M, T>::Trace() const {
  return matrix_funcs::Trace(*this);
}

template <size_t N, size_t M, typename T>
const T& Matrix<N, M, T>::operator()(size_t index_i, size_t index_j) const {
  return data_[index_i][index_j];
};

template <size_t N, size_t M, typename T>
T& Matrix<N, M, T>::operator()(size_t index_i, size_t index_j) {
  return data_[index_i][index_j];
};

template <size_t N, size_t M, typename T>
Matrix<M, N, T> Matrix<N, M, T>::Transposed() {
  Matrix<M, N, T> matrix;
  for (size_t i = 0; i < M; ++i) {
    for (size_t j = 0; j < N; ++j) {
      matrix(i, j) = data_[j][i];
    }
  }
  return matrix;
}

// реализация мат операций из класса
template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator*=(const T& elem) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      data_[i][j] *= elem;
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator+=(const Matrix<N, M, T>& second) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      data_[i][j] += second.data_[i][j];
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator-=(const Matrix<N, M, T>& second) {
  for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < M; ++j) {
      data_[i][j] -= second.data_[i][j];
    }
  }
  return *this;
}

//  мат операции
template <size_t N, size_t M, typename T>
Matrix<N, M, T> operator+(const Matrix<N, M, T>& first,
                          const Matrix<N, M, T>& second) {
  Matrix<N, M, T> matrix(first);
  matrix += second;
  return matrix;
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T> operator-(const Matrix<N, M, T>& first,
                          const Matrix<N, M, T>& second) {
  Matrix<N, M, T> matrix(first);
  matrix -= second;
  return matrix;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T> operator*(const Matrix<N, M, T>& matrix, const T& element) {
  Matrix<N, M, T> new_matrix(matrix);
  new_matrix *= element;
  return new_matrix;
};

template <size_t N, size_t M, size_t K, typename T>
Matrix<N, K, T> operator*(const Matrix<N, M, T>& first,
                          const Matrix<M, K, T>& second) {
  Matrix<N, K, T> matrix;
  for (size_t i = 0; i < N; ++i) {
    for (size_t l = 0; l < M; ++l) {
      for (size_t j = 0; j < K; ++j) {
        matrix(i, j) += first(i, l) * second(l, j);
      }
    }
  }
  return matrix;
};

template <size_t N, size_t M, size_t K, typename T>
bool operator==(const Matrix<N, M, T>& first, const Matrix<N, M, T>& second) {
  return first.Data() == second.Data();
}