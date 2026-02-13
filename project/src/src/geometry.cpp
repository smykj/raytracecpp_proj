#include "geometry.hpp"

double vec4d::dot(vec4d b) {
  return (x * b.x) + (y * b.y) + (z * b.z) + (w * b.w);
}

vec4d mat4d::operator*(vec4d v) {
  return {a.dot(v), b.dot(v), c.dot(v), d.dot(v)};
}

point point::zero() { return {0, 0, 0}; }

point point::operator*(mat4d m) {
  vec4d v{x, y, z, 1};
  vec4d res = m * v;
  return {res.x / res.w, res.y / res.w, res.z / res.w};
}

vector point::operator-(point b) { return {x - b.x, y - b.y, z - b.z}; }

point point::operator+(vector v) { return {x + v.x, y + v.y, z + v.z}; }

vector vector::zero() { return {0, 0, 0}; }

vector vector::normalized() { return *this / this->length(); }

void vector::normalize() { *this = this->normalized(); }

double vector::length() { return vector::dot(*this, *this); }

vector vector::operator*(mat4d m) {
  vec4d v{x, y, z, 0};
  vec4d res = m * v;
  return {res.x, res.y, res.z};
}

vector vector::operator*(double d) { return {x * d, y * d, z * d}; }

vector vector::operator*(int i) { return {x * i, y * i, z * i}; }

vector vector::operator/(int i) { return *this / (double)i; }

vector vector::operator/(double d) { return *this * (1 / d); }

vector vector::operator-(vector b) { return {x - b.x, y - b.y, z - b.z}; }

vector vector::operator-() { return {-x, -y, -z}; }

vector vector::operator+(vector b) { return {x + b.x, y + b.y, z + b.z}; }

double vector::dot(vector a, vector b) {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

double vector::dot(vector a, point b) {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

ray &ray::transform(mat4d transformation) { // should probably be void
  origin = origin * transformation;
  direction = direction * transformation;
  direction.normalize();
  return *this;
}

ray ray::transformed(mat4d transformation) {
  ray result{origin * transformation, direction * transformation};
  result.direction.normalize();
  return result;
}
