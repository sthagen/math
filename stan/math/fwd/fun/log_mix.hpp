#ifndef STAN_MATH_FWD_FUN_LOG_MIX_HPP
#define STAN_MATH_FWD_FUN_LOG_MIX_HPP

#include <stan/math/fwd/meta.hpp>
#include <stan/math/fwd/core.hpp>
#include <stan/math/fwd/fun/exp.hpp>
#include <stan/math/fwd/fun/value_of.hpp>
#include <stan/math/prim/fun/log_mix.hpp>
#include <cmath>
#include <type_traits>

namespace stan {
namespace math {

/* Returns an array of size N with partials of log_mix wrt to its
 * parameters instantiated as fvar<T>
 *
 * @tparam T_theta theta scalar type
 * @tparam T_lambda1 lambda_1 scalar type
 * @tparam T_lambda2 lambda_2 scalar type
 *
 * @param[in] N output array size
 * @param[in] theta_d mixing proportion theta
 * @param[in] lambda1_d log_density with mixing proportion theta
 * @param[in] lambda2_d log_density with mixing proportion 1.0 - theta
 * @param[out] partials_array array of partials derivatives
 */
template <typename T_theta, typename T_lambda1, typename T_lambda2, int N>
inline void log_mix_partial_helper(
    const T_theta& theta, const T_lambda1& lambda1, const T_lambda2& lambda2,
    promote_args_t<T_theta, T_lambda1, T_lambda2> (&partials_array)[N]) {
  using partial_return_type = promote_args_t<T_theta, T_lambda1, T_lambda2>;
  auto lam2_m_lam1 = lambda2 - lambda1;
  auto exp_lam2_m_lam1 = exp(lam2_m_lam1);
  auto one_m_exp_lam2_m_lam1 = 1.0 - exp_lam2_m_lam1;
  auto one_m_t = 1.0 - theta;
  auto one_m_t_prod_exp_lam2_m_lam1 = one_m_t * exp_lam2_m_lam1;
  auto t_plus_one_m_t_prod_exp_lam2_m_lam1
      = theta + one_m_t_prod_exp_lam2_m_lam1;
  auto one_d_t_plus_one_m_t_prod_exp_lam2_m_lam1
      = 1.0 / t_plus_one_m_t_prod_exp_lam2_m_lam1;

  unsigned int offset = 0;
  if (std::is_same<T_theta, partial_return_type>::value) {
    partials_array[offset]
        = one_m_exp_lam2_m_lam1 * one_d_t_plus_one_m_t_prod_exp_lam2_m_lam1;
    ++offset;
  }
  if (std::is_same<T_lambda1, partial_return_type>::value) {
    partials_array[offset] = theta * one_d_t_plus_one_m_t_prod_exp_lam2_m_lam1;
    ++offset;
  }
  if (std::is_same<T_lambda2, partial_return_type>::value) {
    partials_array[offset] = one_m_t_prod_exp_lam2_m_lam1
                             * one_d_t_plus_one_m_t_prod_exp_lam2_m_lam1;
  }
}

/**
 * Return the log mixture density with specified mixing proportion
 * and log densities and its derivative at each.
 *
 * \f[
 * \mbox{log\_mix}(\theta, \lambda_1, \lambda_2)
 * = \log \left( \theta \exp(\lambda_1)
   + (1 - \theta) \exp(\lambda_2) \right).
 * \f]
 *
 * \f[
 * \frac{\partial}{\partial \theta}
 * \mbox{log\_mix}(\theta, \lambda_1, \lambda_2)
 * = \dfrac{\exp(\lambda_1) - \exp(\lambda_2)}
 * {\left( \theta \exp(\lambda_1) + (1 - \theta) \exp(\lambda_2) \right)}
 * \f]
 *
 * \f[
 * \frac{\partial}{\partial \lambda_1}
 * \mbox{log\_mix}(\theta, \lambda_1, \lambda_2)
 * = \dfrac{\theta \exp(\lambda_1)}
 * {\left( \theta \exp(\lambda_1) + (1 - \theta) \exp(\lambda_2) \right)}
 * \f]
 *
 * \f[
 * \frac{\partial}{\partial \lambda_2}
 * \mbox{log\_mix}(\theta, \lambda_1, \lambda_2)
 * = \dfrac{\theta \exp(\lambda_2)}
 * {\left( \theta \exp(\lambda_1) + (1 - \theta) \exp(\lambda_2) \right)}
 * \f]
 *
 * @tparam T inner type of the fvar
 * @param[in] theta mixing proportion in [0, 1].
 * @param[in] lambda1 first log density.
 * @param[in] lambda2 second log density.
 *
 * @return log mixture of densities in specified proportion
 */
template <typename T>
inline fvar<T> log_mix(const fvar<T>& theta, const fvar<T>& lambda1,
                       const fvar<T>& lambda2) {
  if (lambda1.val_ > lambda2.val_) {
    fvar<T> partial_deriv_array[3];
    log_mix_partial_helper(theta, lambda1, lambda2, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1.val_, lambda2.val_),
                   theta.d_ * value_of(partial_deriv_array[0])
                       + lambda1.d_ * value_of(partial_deriv_array[1])
                       + lambda2.d_ * value_of(partial_deriv_array[2]));
  } else {
    fvar<T> partial_deriv_array[3];
    log_mix_partial_helper(1.0 - theta, lambda2, lambda1, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1.val_, lambda2.val_),
                   -theta.d_ * value_of(partial_deriv_array[0])
                       + lambda1.d_ * value_of(partial_deriv_array[2])
                       + lambda2.d_ * value_of(partial_deriv_array[1]));
  }
}

template <typename T, typename P, require_all_arithmetic_t<P>* = nullptr>
inline fvar<T> log_mix(const fvar<T>& theta, const fvar<T>& lambda1,
                       P lambda2) {
  if (lambda1.val_ > lambda2) {
    fvar<T> partial_deriv_array[2];
    log_mix_partial_helper(theta, lambda1, lambda2, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1.val_, lambda2),
                   theta.d_ * value_of(partial_deriv_array[0])
                       + lambda1.d_ * value_of(partial_deriv_array[1]));
  } else {
    fvar<T> partial_deriv_array[2];
    log_mix_partial_helper(1.0 - theta, lambda2, lambda1, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1.val_, lambda2),
                   -theta.d_ * value_of(partial_deriv_array[0])
                       + lambda1.d_ * value_of(partial_deriv_array[1]));
  }
}

template <typename T, typename P, require_all_arithmetic_t<P>* = nullptr>
inline fvar<T> log_mix(const fvar<T>& theta, P lambda1,
                       const fvar<T>& lambda2) {
  if (lambda1 > lambda2.val_) {
    fvar<T> partial_deriv_array[2];
    log_mix_partial_helper(theta, lambda1, lambda2, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1, lambda2.val_),
                   theta.d_ * value_of(partial_deriv_array[0])
                       + lambda2.d_ * value_of(partial_deriv_array[1]));
  } else {
    fvar<T> partial_deriv_array[2];
    log_mix_partial_helper(1.0 - theta, lambda2, lambda1, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1, lambda2.val_),
                   -theta.d_ * value_of(partial_deriv_array[0])
                       + lambda2.d_ * value_of(partial_deriv_array[1]));
  }
}

template <typename T, typename P, require_all_arithmetic_t<P>* = nullptr>
inline fvar<T> log_mix(P theta, const fvar<T>& lambda1,
                       const fvar<T>& lambda2) {
  if (lambda1.val_ > lambda2.val_) {
    fvar<T> partial_deriv_array[2];
    log_mix_partial_helper(theta, lambda1, lambda2, partial_deriv_array);
    return fvar<T>(log_mix(theta, lambda1.val_, lambda2.val_),
                   lambda1.d_ * value_of(partial_deriv_array[0])
                       + lambda2.d_ * value_of(partial_deriv_array[1]));
  } else {
    fvar<T> partial_deriv_array[2];
    log_mix_partial_helper(1.0 - theta, lambda2, lambda1, partial_deriv_array);
    return fvar<T>(log_mix(theta, lambda1.val_, lambda2.val_),
                   lambda1.d_ * value_of(partial_deriv_array[1])
                       + lambda2.d_ * value_of(partial_deriv_array[0]));
  }
}

template <typename T, typename P1, typename P2,
          require_all_arithmetic_t<P1, P2>* = nullptr>
inline fvar<T> log_mix(const fvar<T>& theta, P1 lambda1, P2 lambda2) {
  if (lambda1 > lambda2) {
    fvar<T> partial_deriv_array[1];
    log_mix_partial_helper(theta, lambda1, lambda2, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1, lambda2),
                   theta.d_ * value_of(partial_deriv_array[0]));
  } else {
    fvar<T> partial_deriv_array[1];
    log_mix_partial_helper(1.0 - theta, lambda2, lambda1, partial_deriv_array);
    return fvar<T>(log_mix(theta.val_, lambda1, lambda2),
                   -theta.d_ * value_of(partial_deriv_array[0]));
  }
}

template <typename T, typename P1, typename P2,
          require_all_arithmetic_t<P1, P2>* = nullptr>
inline fvar<T> log_mix(P1 theta, const fvar<T>& lambda1, P2 lambda2) {
  if (lambda1.val_ > lambda2) {
    fvar<T> partial_deriv_array[1];
    log_mix_partial_helper(theta, lambda1, lambda2, partial_deriv_array);
    return fvar<T>(log_mix(theta, lambda1.val_, lambda2),
                   lambda1.d_ * value_of(partial_deriv_array[0]));
  } else {
    fvar<T> partial_deriv_array[1];
    log_mix_partial_helper(1.0 - theta, lambda2, lambda1, partial_deriv_array);
    return fvar<T>(log_mix(theta, lambda1.val_, lambda2),
                   lambda1.d_ * value_of(partial_deriv_array[0]));
  }
}

template <typename T, typename P1, typename P2,
          require_all_arithmetic_t<P1, P2>* = nullptr>
inline fvar<T> log_mix(P1 theta, P2 lambda1, const fvar<T>& lambda2) {
  if (lambda1 > lambda2.val_) {
    fvar<T> partial_deriv_array[1];
    log_mix_partial_helper(theta, lambda1, lambda2, partial_deriv_array);
    return fvar<T>(log_mix(theta, lambda1, lambda2.val_),
                   lambda2.d_ * value_of(partial_deriv_array[0]));
  } else {
    fvar<T> partial_deriv_array[1];
    log_mix_partial_helper(1.0 - theta, lambda2, lambda1, partial_deriv_array);
    return fvar<T>(log_mix(theta, lambda1, lambda2.val_),
                   lambda2.d_ * value_of(partial_deriv_array[0]));
  }
}
}  // namespace math
}  // namespace stan
#endif
