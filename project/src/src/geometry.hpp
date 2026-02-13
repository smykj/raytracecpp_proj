#pragma once

// #include "mymathlib.hpp"

struct point;

struct vec4d {
  double x, y, z, w;
  double dot(vec4d b);
};

struct mat4d {
  vec4d a, b, c, d;
  vec4d operator*(vec4d v);
};

struct vec2i {
  int x, y;
};

struct vector {
  double x, y, z;
  static vector zero();
  vector normalized();
  void normalize();
  double length();
  vector operator*(mat4d m);
  vector operator*(double d);
  vector operator*(int i);
  vector operator/(int i);
  vector operator/(double d);
  vector operator-(vector b);
  vector operator-();
  vector operator+(vector b);
  static double dot(vector a, vector b);
  static double dot(vector a, point b);
};

struct point {
  double x, y, z;
  static point zero();
  point operator*(mat4d m);
  vector operator-(point b);
  point operator+(vector v);
};

struct ray {
  point origin;
  vector direction;
  ray &transform(mat4d transformation);
  ray transformed(mat4d transformation);
};
