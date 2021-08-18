#ifndef STAN_MATH_PRIM_FUN_CHOLESKY_CORR_CONSTRAIN_HPP
#define STAN_MATH_PRIM_FUN_CHOLESKY_CORR_CONSTRAIN_HPP

#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/fun/log1m.hpp>
#include <stan/math/prim/fun/sqrt.hpp>
#include <stan/math/prim/fun/square.hpp>
#include <stan/math/prim/fun/corr_constrain.hpp>
#include <cmath>

namespace stan {
namespace math {

template <typename EigVec, require_eigen_col_vector_t<EigVec>* = nullptr>
inline Eigen::Matrix<value_type_t<EigVec>, Eigen::Dynamic, Eigen::Dynamic>
cholesky_corr_constrain(const EigVec& y, int K) {
  using Eigen::Dynamic;
  using Eigen::Matrix;
  using std::sqrt;
  using T_scalar = value_type_t<EigVec>;
  int k_choose_2 = (K * (K - 1)) / 2;
  check_size_match("cholesky_corr_constrain", "y.size()", y.size(),
                   "k_choose_2", k_choose_2);
  Matrix<T_scalar, Dynamic, 1> z = corr_constrain(y);
  Matrix<T_scalar, Dynamic, Dynamic> x(K, K);
  if (K == 0) {
    return x;
  }
  x.setZero();
  x.coeffRef(0, 0) = 1;
  int k = 0;
  for (int i = 1; i < K; ++i) {
    x.coeffRef(i, 0) = z.coeff(k++);
    T_scalar sum_sqs = square(x.coeff(i, 0));
    for (int j = 1; j < i; ++j) {
      x.coeffRef(i, j) = z.coeff(k++) * sqrt(1.0 - sum_sqs);
      sum_sqs += square(x.coeff(i, j));
    }
    x.coeffRef(i, i) = sqrt(1.0 - sum_sqs);
  }
  return x;
}

// FIXME to match above after debugged
template <typename EigVec, require_eigen_vector_t<EigVec>* = nullptr>
inline Eigen::Matrix<value_type_t<EigVec>, Eigen::Dynamic, Eigen::Dynamic>
cholesky_corr_constrain(const EigVec& y, int K, scalar_type_t<EigVec>& lp) {
  using Eigen::Dynamic;
  using Eigen::Matrix;
  using std::sqrt;
  using T_scalar = value_type_t<EigVec>;
  int k_choose_2 = (K * (K - 1)) / 2;
  check_size_match("cholesky_corr_constrain", "y.size()", y.size(),
                   "k_choose_2", k_choose_2);
  Matrix<T_scalar, Dynamic, 1> z = corr_constrain(y, lp);
  Matrix<T_scalar, Dynamic, Dynamic> x(K, K);
  if (K == 0) {
    return x;
  }
  x.setZero();
  x.coeffRef(0, 0) = 1;
  int k = 0;
  for (int i = 1; i < K; ++i) {
    x.coeffRef(i, 0) = z.coeff(k++);
    T_scalar sum_sqs = square(x.coeff(i, 0));
    for (int j = 1; j < i; ++j) {
      lp += 0.5 * log1m(sum_sqs);
      x.coeffRef(i, j) = z.coeff(k++) * sqrt(1.0 - sum_sqs);
      sum_sqs += square(x.coeff(i, j));
    }
    x.coeffRef(i, i) = sqrt(1.0 - sum_sqs);
  }
  return x;
}

/**
 * Return The cholesky of a `KxK` correlation matrix.
 * @tparam Jacobian If true, incremented `lp` with the log Jacobian
 * @tparam T A type inheriting from `Eigen::DenseBase` or a `var_value` with
 *  inner type inheriting from `Eigen::DenseBase` with compile time dynamic rows
 *  and 1 column.
 * @param y Linearly Serialized vector of size `(K * (K - 1))/2` holding the
 *  column major order elements of the lower triangurlar.
 * @param K The size of the matrix to return.
 * @param lp Log probability that is incremented with the log Jacobian
 */
template <bool Jacobian, typename T>
inline auto cholesky_corr_constrain(const T& y, int K, scalar_type_t<T>& lp) {
  if (Jacobian) {
    return cholesky_corr_constrain(y, K, lp);
  } else {
    return cholesky_corr_constrain(y, K);
  }
}
}  // namespace math
}  // namespace stan
#endif
