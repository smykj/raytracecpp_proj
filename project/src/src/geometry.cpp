#include "geometry.hpp"
#include <cmath>
namespace raytracer {
double vec4d_t::dot(vec4d_t b) {
  return (vec[0] * b.vec[0]) + (vec[1] * b.vec[1]) + (vec[2] * b.vec[2]) +
         (vec[3] * b.vec[3]);
}

vec4d_t mat4d_t::operator*(vec4d_t v) {
  return {mat[0].dot(v), mat[1].dot(v), mat[2].dot(v), mat[3].dot(v)};
}

mat4d_t mat4d_t::inverted() {

  mat4d_t result;
  double s0 = (mat[0][0] * mat[1][1]) - (mat[1][0] * mat[0][1]);
  double s1 = (mat[0][0] * mat[1][2]) - (mat[1][0] * mat[0][2]);
  double s2 = (mat[0][0] * mat[1][3]) - (mat[1][0] * mat[0][3]);
  double s3 = (mat[0][1] * mat[1][2]) - (mat[1][1] * mat[0][2]);
  double s4 = (mat[0][1] * mat[1][3]) - (mat[1][1] * mat[0][3]);
  double s5 = (mat[0][2] * mat[1][3]) - (mat[1][2] * mat[0][3]);

  double c0 = (mat[0][0] * mat[1][1]) - (mat[1][0] * mat[0][1]);
  double c1 = (mat[0][0] * mat[1][2]) - (mat[1][0] * mat[0][2]);
  double c2 = (mat[0][0] * mat[1][3]) - (mat[1][0] * mat[0][3]);
  double c3 = (mat[0][1] * mat[1][2]) - (mat[1][1] * mat[0][2]);
  double c4 = (mat[0][1] * mat[1][3]) - (mat[1][1] * mat[0][3]);
  double c5 = (mat[0][2] * mat[1][3]) - (mat[1][2] * mat[0][3]);

  double det =
      (s0 * c5) - (s1 * c4) + (s2 * c3) + (s3 * c2) - (s4 * c1) + (s5 * c0);
  if (std::fabs(det) < 0.00001) {
    return result; // assuming it to be 0-initialized, todo: check
  }

  double inv = 1.0 / det;
  result[0][0] = ((mat[1][1] * c5) - (mat[1][2] * c4) + (mat[1][3] * c3)) * inv;
  result[0][1] =
      ((-mat[0][1] * c5) + (mat[0][2] * c4) - (mat[0][3] * c3)) * inv;
  result[0][2] = ((mat[3][1] * s5) - (mat[3][2] * s4) + (mat[3][3] * s3)) * inv;
  result[0][3] =
      ((-mat[2][1] * s5) + (mat[2][2] * s4) - (mat[2][3] * s3)) * inv;
  result[1][0] =
      ((-mat[1][0] * c5) + (mat[1][2] * c2) - (mat[1][3] * c1)) * inv;
  result[1][1] = ((mat[0][0] * c5) - (mat[0][2] * c2) + (mat[0][3] * c1)) * inv;
  result[1][2] =
      ((-mat[3][0] * s5) + (mat[3][2] * s2) - (mat[3][3] * s1)) * inv;
  result[1][3] = ((mat[2][0] * s5) - (mat[2][2] * s2) + (mat[2][3] * s1)) * inv;
  result[2][0] = ((mat[1][0] * c4) - (mat[1][1] * c2) + (mat[1][3] * c0)) * inv;
  result[2][1] =
      ((-mat[0][0] * c4) + (mat[0][1] * c2) - (mat[0][3] * c0)) * inv;
  result[2][2] = ((mat[3][0] * s4) - (mat[3][1] * s2) + (mat[3][3] * s0)) * inv;
  result[2][3] =
      ((-mat[2][0] * s4) + (mat[2][1] * s2) - (mat[2][3] * s0)) * inv;
  result[3][0] =
      ((-mat[1][0] * c3) + (mat[1][1] * c1) - (mat[1][2] * c0)) * inv;
  result[3][1] = ((mat[0][0] * c3) - (mat[0][1] * c1) + (mat[0][2] * c0)) * inv;
  result[3][2] =
      ((-mat[3][0] * s3) + (mat[3][1] * s1) - (mat[3][2] * s0)) * inv;
  result[3][3] = ((mat[2][0] * s3) - (mat[2][1] * s1) + (mat[2][2] * s0)) * inv;
  return result;
}

mat4d_t mat4d_t::axis_angle(vector_t axis, double angle) {
  mat4d_t result; // todo assuming 0 initialization
  axis.normalize();
  double cos = std::cos(-angle);
  double sin = std::sin(-angle);
  double t = 1.0 - cos;
  double tXX = t * axis.x * axis.x;
  double tXY = t * axis.x * axis.y;
  double tXZ = t * axis.x * axis.z;
  double tYY = t * axis.y * axis.y;
  double tYZ = t * axis.y * axis.z;
  double tZZ = t * axis.z * axis.z;
  double sinX = sin * axis.x;
  double sinY = sin * axis.y;
  double sinZ = sin * axis.z;

  result[0][0] = tXX + cos;
  result[0][1] = tXY - sinZ;
  result[0][2] = tXZ + sinY;
  result[1][0] = tXY + sinZ;
  result[1][1] = tYY + cos;
  result[1][2] = tYZ - sinX;
  result[2][0] = tXZ - sinY;
  result[2][1] = tYZ + sinX;
  result[2][2] = tZZ + cos;
  result[3][3] = 1;
  return result;
}
point_t point_t::zero() { return {0, 0, 0}; }

point_t point_t::operator*(mat4d_t m) {
  vec4d_t v{x, y, z, 1};
  vec4d_t res = m * v;
  return {res.vec[0] / res.vec[3], res.vec[1] / res.vec[3],
          res.vec[2] / res.vec[3]};
}

vector_t point_t::operator-(point_t b) const {
  return {x - b.x, y - b.y, z - b.z};
}

point_t point_t::operator+(vector_t v) { return {x + v.x, y + v.y, z + v.z}; }

vector_t vector_t::zero() { return {0, 0, 0}; }

vector_t vector_t::normalized() { return *this / this->length(); }

void vector_t::normalize() { *this = this->normalized(); }

double vector_t::length() const { return vector_t::dot(*this, *this); }

vector_t vector_t::operator*(mat4d_t m) {
  vec4d_t v{x, y, z, 0};
  vec4d_t res = m * v;
  return {res.vec[0], res.vec[1], res.vec[2]};
}

vector_t vector_t::operator*(double d) { return {x * d, y * d, z * d}; }

vector_t vector_t::operator*(int i) { return {x * i, y * i, z * i}; }

vector_t vector_t::operator/(int i) { return *this / (double)i; }

vector_t vector_t::operator/(double d) { return *this * (1 / d); }

vector_t vector_t::operator-(vector_t b) { return {x - b.x, y - b.y, z - b.z}; }

vector_t vector_t::operator-() { return {-x, -y, -z}; }

vector_t vector_t::operator+(vector_t b) { return {x + b.x, y + b.y, z + b.z}; }

double vector_t::dot(vector_t a, vector_t b) {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

double vector_t::dot(vector_t a, point_t b) {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

ray_t &ray_t::transform(mat4d_t transformation) { // should probably be void
  origin = origin * transformation;
  direction = direction * transformation;
  direction.normalize();
  return *this;
}

ray_t ray_t::transformed(mat4d_t transformation) {
  ray_t result{origin * transformation, direction * transformation};
  result.direction.normalize();
  return result;
}

mat4d_t mat4d_t::create_translation(vector_t vec) {

  mat4d_t result = identity();
  result[3] = vec4d_t{vec.x, vec.y, vec.z, 1};
  return result;
}

} // namespace raytracer
