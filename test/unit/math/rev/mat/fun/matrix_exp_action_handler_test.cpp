#include <stan/math/rev/mat.hpp>
#include <gtest/gtest.h>
// #include <stan/math/prim/scal/err/check_not_nan.hpp>
#include <test/unit/math/rev/mat/fun/util.hpp>
#include <test/unit/math/rev/mat/util.hpp>
#include <stan/math/rev/mat/fun/matrix_exp_action_handler.hpp>
#include <stan/math/rev/mat/fun/to_var.hpp>
#include <vector>

TEST(MathMatrix, matrix_exp_action_diag) {
  using MatrixType = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
  using VectorType = Eigen::Matrix<double, Eigen::Dynamic, 1>;
  stan::math::matrix_exp_action_handler handler;

  {
    double t = 0.5;
    MatrixType m1(2, 2);
    VectorType b(2);
    m1 << 2, 0, 0, 2;
    b << 1, 1;
    auto res = handler.action(m1, b, t);
    EXPECT_NEAR(res(0), M_E, 1.e-8);
    EXPECT_NEAR(res(1), M_E, 1.e-8);
  }

  {
    double t = 1.0;
    MatrixType m1(2, 2);
    VectorType b = VectorType::Random(2);
    m1 << 1, 0, 0, 2;
    auto res = handler.action(m1, b, t);
    EXPECT_NEAR(res(0), b(0) * M_E, 1.e-8);
    EXPECT_NEAR(res(1), b(1) * M_E * M_E, 1.e-8);
  }

  {
    double t = 1.0;
    std::srand(1299);
    MatrixType m1(2, 2);
    VectorType b = VectorType::Random(2);
    m1 << -4.0, 0, 0, -5.0;
    auto res = handler.action(m1, b, t);
    EXPECT_NEAR(res(0), b(0) / (M_E * M_E * M_E * M_E), 1.e-8);
    EXPECT_NEAR(res(1), b(1) / (M_E * M_E * M_E * M_E * M_E), 1.e-8);
  }

  {
    std::srand(999);
    double t = static_cast<double>((std::rand()) / RAND_MAX);
    VectorType b = VectorType::Random(5);
    VectorType d = VectorType::Random(5);
    MatrixType m = d.asDiagonal();
    auto res = handler.action(m, b, t);
    for (int i = 0; i < 5; ++i) {
      EXPECT_NEAR(res(i), b(i) * std::exp(t * d(i)), 1.e-8);
    }
  }
}

TEST(MathMatrix, matrix_exp_action_vector) {
  using MatrixType = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
  using VectorType = Eigen::Matrix<double, Eigen::Dynamic, 1>;
  stan::math::matrix_exp_action_handler handler;
  std::srand(999);

  for (size_t n = 2; n < 10; ++n) {
    MatrixType A = MatrixType::Random(n, n);
    VectorType b = VectorType::Random(n);
    VectorType res = handler.action(A, b);
    MatrixType expA = stan::math::matrix_exp(A);
    VectorType expb = expA * b;
    for (size_t i = 0; i < n; ++i) {
      EXPECT_NEAR(res(i), expb(i), 1.e-6);
    }

    int m1, s1, m2, s2;
    const double t1 = 9.9, t2 = 1.0;
    handler.set_approximation_parameter(A, t1, m1, s1);
    A *= t1;
    handler.set_approximation_parameter(A, t2, m2, s2);
    EXPECT_EQ(m1, m2);
    EXPECT_EQ(s1, s2);
  }
}

TEST(MathMatrix, matrix_exp_action_matrix) {
  using MatrixType = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
  stan::math::matrix_exp_action_handler handler;
  std::srand(999);

  constexpr int N = 10;
  constexpr int M = 4;
  Eigen::Matrix<double, N, N> A = Eigen::Matrix<double, N, N>::Random();
  Eigen::Matrix<double, N, M> B = Eigen::Matrix<double, N, M>::Random();

  Eigen::Matrix<double, N, M> res = handler.action(A, B);
  MatrixType Ad(A);
  MatrixType expa = stan::math::matrix_exp(Ad);
  Eigen::Matrix<double, N, M> expb = expa * B;

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; ++j) {
      EXPECT_FLOAT_EQ(res(i, j), expb(i, j));
    }
  }
}

TEST(MathMatrix, matrix_exp_action_matrix_transpose) {
  using MatrixType = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
  stan::math::matrix_exp_action_handler handler;
  std::srand(1999);

  constexpr int N = 10;
  constexpr int M = 4;
  Eigen::Matrix<double, N, N> A = Eigen::Matrix<double, N, N>::Random();
  Eigen::Matrix<double, N, M> B = Eigen::Matrix<double, N, M>::Random();

  Eigen::Matrix<double, N, M> res = handler.action(A.transpose(), B);
  MatrixType Ad(A);
  MatrixType expa = stan::math::matrix_exp(Ad).transpose();
  Eigen::Matrix<double, N, M> expb = expa * B;

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; ++j) {
      EXPECT_NEAR(res(i, j), expb(i, j), 1.e-6);
    }
  }
}
