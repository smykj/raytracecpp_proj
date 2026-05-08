#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
namespace raytracer {
struct point_t;
struct vector_t;
// typedef std::array<double, 3> color_t;
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

  static mat4d_t axis_angle(vector_t axis, double angle);

  static mat4d_t scale(double scale) {
    mat4d_t result; // todo: again assuming its 0-initialized;
    result[0][0] = scale;
    result[1][1] = scale;
    result[2][2] = scale;
    result[3][3] = 1;
    return result;
  }

  static mat4d_t create_translation(vector_t vec);
};

struct vec2i_t {
  int x, y;
};

struct vector_t {
  double x, y, z;
  // vector_t(std::array<double, 3> val) {
  //   x = val[0];
  //   y = val[1];
  //   z = val[2];
  // }
  // vector_t() = default;
  static vector_t zero();
  vector_t normalized();
  void normalize();
  double length() const;
  vector_t operator*(mat4d_t m);
  vector_t operator*(double d);
  vector_t operator*(int i);
  vector_t operator*(size_t i);
  vector_t operator/(int i);
  vector_t operator/(double d);
  vector_t operator-(vector_t b);
  vector_t operator-();
  vector_t operator+(vector_t b);
  static double dot(vector_t a, vector_t b);
  static double dot(vector_t a, point_t b);
  static vector_t cross(vector_t a, vector_t b) {
    vector_t result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);
    return result;
  }
  auto &operator*=(const mat4d_t rhs) {
    *this = *this * rhs;
    return *this;
  }

  bool operator==(const vector_t rhs) const {
    return rhs.x == this->x && rhs.y == this->y && rhs.z == this->z;
  }

  static double angle(const vector_t a, const vector_t b) {
    auto temp = dot(a, b);
    return std::acos(std::clamp(temp / (a.length() * b.length()), -1.0, 1.0));
  }
};

struct point_t {
  double x, y, z;
  static point_t zero();
  point_t operator*(mat4d_t m);
  vector_t operator-(point_t b) const;
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
