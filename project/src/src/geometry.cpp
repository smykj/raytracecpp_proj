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
  double a = mat[0][0];
  double b = mat[1][0];
  double c = mat[2][0];
  double d = mat[3][0];
  double e = mat[0][1];
  double f = mat[1][1];
  double g = mat[2][1];
  double h = mat[3][1];
  double i = mat[0][2];
  double j = mat[1][2];
  double k = mat[2][2];
  double l = mat[3][2];
  double m = mat[0][3];
  double n = mat[1][3];
  double o = mat[2][3];
  double p = mat[3][3];

  double kp_lo = (k * p) - (l * o);
  double jp_ln = (j * p) - (l * n);
  double jo_kn = (j * o) - (k * n);
  double ip_lm = (i * p) - (l * m);
  double io_km = (i * o) - (k * m);
  double in_jm = (i * n) - (j * m);

  double a11 = +((f * kp_lo) - (g * jp_ln) + (h * jo_kn));
  double a12 = -((e * kp_lo) - (g * ip_lm) + (h * io_km));
  double a13 = +((e * jp_ln) - (f * ip_lm) + (h * in_jm));
  double a14 = -((e * jo_kn) - (f * io_km) + (g * in_jm));

  double det = (a * a11) + (b * a12) + (c * a13) + (d * a14);

  double invDet = 1.0 / det;

  result[0] = vec4d_t{a11, a12, a13, a14} * invDet;

  result[1] = vec4d_t{-((b * kp_lo) - (c * jp_ln) + (d * jo_kn)),
                      +((a * kp_lo) - (c * ip_lm) + (d * io_km)),
                      -((a * jp_ln) - (b * ip_lm) + (d * in_jm)),
                      +((a * jo_kn) - (b * io_km) + (c * in_jm))} *
              invDet;

  double gp_ho = (g * p) - (h * o);
  double fp_hn = (f * p) - (h * n);
  double fo_gn = (f * o) - (g * n);
  double ep_hm = (e * p) - (h * m);
  double eo_gm = (e * o) - (g * m);
  double en_fm = (e * n) - (f * m);

  result[2] = vec4d_t{+((b * gp_ho) - (c * fp_hn) + (d * fo_gn)),
                      -((a * gp_ho) - (c * ep_hm) + (d * eo_gm)),
                      +((a * fp_hn) - (b * ep_hm) + (d * en_fm)),
                      -((a * fo_gn) - (b * eo_gm) + (c * en_fm))} *
              invDet;

  double gl_hk = (g * l) - (h * k);
  double fl_hj = (f * l) - (h * j);
  double fk_gj = (f * k) - (g * j);
  double el_hi = (e * l) - (h * i);
  double ek_gi = (e * k) - (g * i);
  double ej_fi = (e * j) - (f * i);

  result[3] = vec4d_t{-((b * gl_hk) - (c * fl_hj) + (d * fk_gj)),
                      +((a * gl_hk) - (c * el_hi) + (d * ek_gi)),
                      -((a * fl_hj) - (b * el_hi) + (d * ej_fi)),
                      +((a * fk_gj) - (b * ek_gi) + (c * ej_fi))} *
              invDet;
  return result;
}

mat4d_t mat4d_t::axis_angle(vector_t axis, double angle) {
  mat4d_t result = mat4d_t::identity();
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

double vector_t::length() const {
  return std::sqrt(vector_t::dot(*this, *this));
}

vector_t vector_t::operator*(mat4d_t m) {
  vec4d_t v{x, y, z, 0};
  vec4d_t res = m * v;
  return {res.vec[0], res.vec[1], res.vec[2]};
}

// vector_t vector_t::operator*(double d) { return {x * d, y * d, z * d}; }

// vector_t vector_t::operator*(int i) { return {x * i, y * i, z * i}; }

// template <typename T> vector_t vector_t::operator*(T i) {
// return {x * i, y * i, z * i};
// }
// vector_t vector_t::operator/(int i) { return *this / (double)i; }

// vector_t vector_t::operator/(double d) { return *this * (1 / d); }

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
  return result.transposed();
}

} // namespace raytracer
