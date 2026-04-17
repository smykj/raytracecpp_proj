#pragma once

// #include "mymathlib.hpp"

#include <array>
#include <cstddef>
namespace raytracer {
struct point_t;
typedef std::array<double, 3> color_t;
struct vec4d_t {
  // double x, y, z, w;
  std::array<double, 4> vec;
  double dot(vec4d_t b);
  // auto &operator[](this auto &self, size_t i) {
  //   return self.vec[i];
  // } // todo: understand this better
  const auto &operator[](size_t i) const { return vec[i]; }
  auto &operator[](size_t i) { return vec[i]; }
};

struct mat4d_t {
  // vec4d_t a, b, c, d;
  std::array<vec4d_t, 4> mat;
  vec4d_t operator*(vec4d_t v);
  mat4d_t inverted();
  const auto &operator[](size_t i) const { return mat[i]; }
  auto &operator[](size_t i) { return mat[i]; }
  auto operator*(const mat4d_t rhs) const {

    double num;
    mat4d_t result;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        num = 0;
        for (int k = 0; k < 4; k++) {
          num += (*this)[i][k] * rhs[k][j];
        }
        result[i][j] = num;
      }
    }

    return result;
  }
  auto &operator*=(const mat4d_t rhs) {
    *this = *this * rhs;
    return *this;
  }
  static mat4d_t identity() {
    mat4d_t result; // todo: again assuming its 0-initialized;
    result[0][0] = 1;
    result[1][1] = 1;
    result[2][2] = 1;
    result[3][3] = 1;
    return result;
  }
};

struct vec2i_t {
  int x, y;
};

struct vector_t {
  double x, y, z;
  static vector_t zero();
  vector_t normalized();
  void normalize();
  double length();
  vector_t operator*(mat4d_t m);
  vector_t operator*(double d);
  vector_t operator*(int i);
  vector_t operator/(int i);
  vector_t operator/(double d);
  vector_t operator-(vector_t b);
  vector_t operator-();
  vector_t operator+(vector_t b);
  static double dot(vector_t a, vector_t b);
  static double dot(vector_t a, point_t b);
  auto &operator*=(const mat4d_t rhs) {
    *this = *this * rhs;
    return *this;
  }
};

struct point_t {
  double x, y, z;
  static point_t zero();
  point_t operator*(mat4d_t m);
  vector_t operator-(point_t b);
  point_t operator+(vector_t v);
  auto &operator*=(const mat4d_t rhs) {
    *this = *this * rhs;
    return *this;
  }
};

struct ray_t {
  point_t origin;
  vector_t direction;
  ray_t &transform(mat4d_t transformation);
  ray_t transformed(mat4d_t transformation);
};
} // namespace raytracer
