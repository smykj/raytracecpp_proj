#pragma once
#include "geometry.hpp"
// #include "scene_hierarchy.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <numbers>
#include <tgmath.h>
#include <utility>
#include <vector>
namespace raytracer {
class SceneHierarchy;
// todo: add pragma once everywhere

struct color_t {
  std::array<double, 3> arr;
  const auto &operator[](size_t i) const { return arr[i]; }
  auto &operator[](size_t i) { return arr[i]; }
  color_t operator+(color_t rhs) const {
    return color_t{(*this)[0] + rhs[0], (*this)[1] + rhs[1],
                   (*this)[2] + rhs[2]};
  }
  color_t operator*(color_t rhs) const {
    return color_t{(*this)[0] * rhs[0], (*this)[1] * rhs[1],
                   (*this)[2] * rhs[2]};
  }
  color_t operator*(double scalar) const {
    return color_t{(*this)[0] * scalar, (*this)[1] * scalar,
                   (*this)[2] * scalar};
  }
};
struct Light {
  color_t intensity;
  color_t color;
  point_t position;
};
class BRDF {
public:
  double transparency;
  double critical_angle;
  double specular_coefficient;
  color_t color;
  double ambient_coefficient;
  double diffuse_coefficient;
  virtual ~BRDF() noexcept = default;
  BRDF() = default;
  void SetRefractionIndex(double ri) {
    refraction_index = ri;
    critical_angle =
        asin(refraction_index > 1 ? 1 / refraction_index : refraction_index);
  }
  double GetRefractionIndex() const { return refraction_index; }
  virtual color_t Shade(vector_t normal, vector_t light_dir, vector_t view_dir,
                        const Light &light) = 0;

  BRDF(double ambient_coefficient, double diffuse_coefficient,
       double specular_coefficient, color_t color, double transparency,
       double refraction_index)
      : transparency(transparency), specular_coefficient(specular_coefficient),
        color(color), ambient_coefficient(ambient_coefficient),
        diffuse_coefficient(diffuse_coefficient),
        refraction_index(refraction_index) {
    critical_angle = asin(1 / refraction_index);
  }

private:
  double refraction_index;
};

class Phong : public BRDF {
public:
  ~Phong() noexcept override = default;
  int specular_exponent;
  Phong() = default;
  Phong(double ambient_coefficient, double diffuse_coefficient,
        double specular_coefficient, int specular_exponent, color_t color,
        double transparency, double refraction_index)
      : BRDF(ambient_coefficient, diffuse_coefficient, specular_coefficient,
             color, transparency, refraction_index),
        specular_exponent(specular_exponent) {
    critical_angle = asin(1 / refraction_index);
  }

  color_t Shade(vector_t normal, vector_t light_dir, vector_t view_dir,
                const Light &light) override {
    if (vector_t::dot(normal, light_dir) < 0) {
      return {0, 0, 0};
    }

    color_t result;
    color_t diffuse =
        light.intensity * color * vector_t::dot(light_dir, normal);
    vector_t reflection =
        normal * 2 * vector_t::dot(normal, light_dir) - light_dir;
    reflection.normalize();
    color_t specular;
    if (vector_t::dot(view_dir, reflection) < 0) {
      result = diffuse * diffuse_coefficient;
    } else {
      specular = light.intensity * light.color * specular_coefficient *
                 pow(vector_t::dot(view_dir, reflection), specular_exponent);
      diffuse = diffuse * diffuse_coefficient;
      result = diffuse + specular;
    }
    return result;
  } // todo: refactor after making color_t a normal type
};
class OrenNayar : public BRDF {
public:
  ~OrenNayar() noexcept override = default;
  double roughness;
  double rms;
  OrenNayar() = default;
  OrenNayar(double ambient_coefficient, double diffuse_coefficient,
            double specular_coefficient, color_t color, double transparency,
            double refraction_index, double roughness, double rms)
      : BRDF(ambient_coefficient, diffuse_coefficient, specular_coefficient,
             color, transparency, refraction_index),
        roughness(roughness), rms(rms) {
    critical_angle = asin(1 / refraction_index);
  }

  color_t Shade(vector_t normal, vector_t light_dir, vector_t view_dir,
                const Light &light) override {
    vector_t projectionLD =
        (light_dir - (normal * vector_t::dot(normal, light_dir))).normalized();
    vector_t projectionVD =
        (view_dir - (normal * vector_t::dot(normal, view_dir))).normalized();

    double cos_theta_i = vector_t::dot(normal, light_dir);
    double theta_i = acos(cos_theta_i);
    double theta_o = acos(vector_t::dot(normal, view_dir));
    double alpha = std::max(theta_o, theta_i);
    double beta = std::min(theta_o, theta_i);

    double A = 1 - (0.5 * (roughness / (roughness + 0.33)));
    double B = 0.45 * (roughness / (roughness + 0.09));
    color_t diffuse =
        color * cos_theta_i *
        (A + (B * (std::max(vector_t::dot(projectionLD, projectionVD), 0.0)) *
              sin(alpha) * tan(beta)));
    vector_t half = vector_t::dot(light_dir, view_dir) < 0
                        ? -((light_dir + view_dir).normalized())
                        : (light_dir + view_dir).normalized();
    alpha = acos(vector_t::dot(normal, half));
    double color_scalar = (exp((-pow(tan(alpha), 2)) / (rms * rms))) /
                          (std::numbers::pi * rms * rms * pow(cos(alpha), 4));
    color_t specular = light.color * light.intensity * color_scalar;
    return diffuse * diffuse_coefficient + specular * specular_coefficient;
  } // todo: refactor after making color_t a normal type
};

// todo: when multithreading, make most shared resources const&, for example the
// lights vector
class Solids {
public:
  Solids(std::shared_ptr<BRDF> brdf, const std::vector<Light> &lights)
      : brdf(std::move(brdf)), lights(&lights) {}
  virtual ~Solids() noexcept = default;
  std::shared_ptr<BRDF>
      brdf; // todo: think about which type of reference to use
  const std::vector<Light> *lights;

  bool RayIntersection(ray_t ray, SceneHierarchy *scene_hierarchy,
                       mat4d_t transformation_to_world, color_t &color,
                       int recursion_depth, point_t &intersection);
  virtual bool RawIntersection(ray_t ray, point_t &intersection,
                               vector_t &normal, bool &inside) = 0;
  color_t MultiShade(vector_t normal, SceneHierarchy *scene_hierarchy,
                     point_t intersection, vector_t view_dir);
};

class Sphere : public Solids {
public:
  // todo: think about how to pass the lights vector around
  Sphere(std::shared_ptr<BRDF> brdf, const std::vector<Light> &lights)
      : Solids(std::move(brdf), lights) {}
  bool RawIntersection(ray_t ray, point_t &intersection, vector_t &normal,
                       bool &inside) override {
    vector_t vvec = point_t::zero() - ray.origin;
    double tzero = vector_t::dot(vvec, ray.direction);
    double dist_squared = vector_t::dot(vvec, vvec) - (tzero * tzero);
    double inclination_squared = 1 - dist_squared;

    if (inclination_squared <= 0 || tzero < 0) {
      intersection = point_t::zero();
      normal = vector_t::zero();
      inside = false;
      return false;
    }

    double inclination = sqrt(inclination_squared);

    // checks if the origin is inside unit sphere
    inside = sqrt(pow(ray.origin.x, 2) + pow(ray.origin.y, 2) +
                  pow(ray.origin.z, 2)) < 1.000001;

    double t;
    if (inside) {
      t = (tzero + inclination > tzero - inclination ? tzero + inclination
                                                     : tzero - inclination);
      intersection = ray.origin + ray.direction * t;
      // todo: check if this normal gives correct result, same for inside of the
      // else if block below
      normal = -vector_t(intersection.x, intersection.y, intersection.z)
                    .normalized();
    } else if (tzero + inclination < tzero - inclination) {
      t = tzero + inclination;
      intersection = ray.origin + ray.direction * (tzero + inclination);
      normal =
          vector_t(intersection.x, intersection.y, intersection.z).normalized();
    } else {
      t = tzero - inclination;
      intersection = ray.origin + ray.direction * (tzero - inclination);
      normal =
          vector_t(intersection.x, intersection.y, intersection.z).normalized();
    }

    if (t < 0.0000001) {
      intersection = point_t::zero();
      normal = vector_t::zero();
      inside = false;
      return false;
    }

    return true;
  }
};

class Plane : public Solids {
public:
  Plane(std::shared_ptr<BRDF> brdf, const std::vector<Light> &lights)
      : Solids(brdf, lights) {}

  bool RawIntersection(ray_t ray, point_t &intersection, vector_t &normal,
                       bool &inside) override {
    inside = false;
    normal = vector_t(0, 1, 0);
    if (abs(vector_t::dot(normal, ray.direction)) < 0.0000001) {
      intersection = point_t::zero();
      return false;
    }

    double t = -vector_t::dot(normal, ray.origin) /
               vector_t::dot(normal, ray.direction);

    intersection = ray.origin + ray.direction * t;

    return t >= 0.0000001;
  }
};

} // namespace raytracer
