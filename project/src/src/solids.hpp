#pragma once
#include "geometry.hpp"
#include "scene_hierarchy.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <numbers>
#include <tgmath.h>
#include <vector>
namespace raytracer {
// todo: add pragma once everywhere
color_t multiply_color(color_t lhs, color_t rhs) {
  return {lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2]};
}
color_t add_color(color_t lhs, color_t rhs) {
  return {lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]};
}

color_t multiply_color(color_t lhs, double scalar) {
  return {lhs[0] * scalar, lhs[1] * scalar, lhs[2] * scalar};
} // todo: make color_t an actual type
class Light {
public:
  color_t intensity;
  color_t color;
  point_t position;
}; // todo: remove, include camera
class BRDF {
public:
  double transparency;
  double critical_angle;
  double specular_coefficient;
  color_t color;
  double ambient_coefficient;
  double diffuse_coefficient;
  virtual ~BRDF() noexcept = default;
  void SetRefractionIndex(double ri) {
    refraction_index = ri;
    critical_angle =
        asin(refraction_index > 1 ? 1 / refraction_index : refraction_index);
  }
  double GetRefractionIndex() const { return refraction_index; }
  virtual color_t Shade(vector_t normal, vector_t light_dir, vector_t view_dir,
                        std::shared_ptr<Light> light) = 0;

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
                std::shared_ptr<Light> light) override {
    if (vector_t::dot(normal, light_dir) < 0) {
      return {0, 0, 0};
    }

    color_t result;
    color_t diffuse = multiply_color(multiply_color(light->intensity, color),
                                     vector_t::dot(light_dir, normal));
    vector_t reflection =
        normal * 2 * vector_t::dot(normal, light_dir) - light_dir;
    reflection.normalize();
    color_t specular;
    if (vector_t::dot(view_dir, reflection) < 0) {
      result = multiply_color(diffuse, diffuse_coefficient);
    } else {
      specular = multiply_color(
          multiply_color(light->intensity, light->color),
          specular_coefficient *
              pow(vector_t::dot(view_dir, reflection), specular_exponent));
      diffuse = multiply_color(diffuse, diffuse_coefficient);
      result = add_color(diffuse, specular);
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
                std::shared_ptr<Light> light) override {
    vector_t projectionLD =
        (light_dir - (normal * vector_t::dot(normal, light_dir))).normalized();
    vector_t projectionVD =
        (view_dir - (normal * vector_t::dot(normal, view_dir))).normalized();

    double cos_theta_i = vector_t::dot(normal, light_dir);
    double theta_i = acos(cos_theta_i);
    double theta_o = acos(vector_t::dot(normal, view_dir));
    double alpha = std::max(theta_o, theta_i);
    double beta = std::min(theta_o, theta_i);

    double A = 1 - 0.5 * (roughness / (roughness + 0.33));
    double B = 0.45 * (roughness / (roughness + 0.09));
    color_t diffuse = multiply_color(
        color,
        cos_theta_i *
            (A +
             B * (std::max(vector_t::dot(projectionLD, projectionVD), 0.0)) *
                 sin(alpha) * tan(beta)));
    vector_t half = vector_t::dot(light_dir, view_dir) < 0
                        ? -((light_dir + view_dir).normalized())
                        : (light_dir + view_dir).normalized();
    alpha = acos(vector_t::dot(normal, half));
    double color_scalar = (exp((-pow(tan(alpha), 2)) / (rms * rms))) /
                          (std::numbers::pi * rms * rms * pow(cos(alpha), 4));
    color_t specular = multiply_color(
        multiply_color(light->color, light->intensity), color_scalar);
    return add_color(multiply_color(diffuse, diffuse_coefficient),
                     multiply_color(specular, specular_coefficient));
  } // todo: refactor after making color_t a normal type
};

// todo: when multithreading, make most shared resources const&, for example the
// lights vector
class Solids {
public:
  Solids(std::shared_ptr<BRDF> brdf, std::shared_ptr<std::vector<Light>> lights)
      : brdf(brdf), lights(lights) {}
  virtual ~Solids() noexcept = default;
  std::shared_ptr<BRDF>
      brdf; // todo: think about which type of reference to use
  std::shared_ptr<std::vector<Light>> lights;

  bool RayIntersection(ray_t ray, SceneHierarchy *scene_hierarchy,
                       mat4d_t transformation_to_world, color_t &color,
                       int recursion_depth, point_t &intersection) {
    bool inside = false;
    vector_t normal;
    if (!RawIntersection(ray, intersection, normal, inside)) {
      color = {0, 0, 0};
      return false;
    }
    if (recursion_depth == -1) { // means that color is irrelevant, just checks
                                 // whether there was a hit
      color = {0, 0, 0};
      return true;
    }

    intersection *= transformation_to_world;
    normal *= transformation_to_world;
    normal.normalize();
    ray_t myray = ray.transformed(transformation_to_world);
    color = MultiShade(normal, scene_hierarchy, intersection, -myray.direction);

    if (recursion_depth < scene_hierarchy->max_recursion_depth) {
      ray_t refl_ray = ray_t(
          intersection, (normal * 2 * vector_t::dot(normal, -myray.direction)) +
                            myray.direction);
      color_t reflection_color = {0, 0, 0};
      color_t refraction_color = {0, 0, 0};

      double incidence_angle = acos(vector_t::dot(normal, -myray.direction));
      double n12 = (inside ? brdf->GetRefractionIndex()
                           : 1 / brdf->GetRefractionIndex());

      if (brdf->specular_coefficient != 0) {
        scene_hierarchy->RaySceneIntersection(refl_ray, reflection_color,
                                              recursion_depth + 1);
      }

      if ((n12 > 1 && incidence_angle > brdf->critical_angle) ||
          brdf->transparency == 0) {
        color = add_color(
            multiply_color(color, (1 - brdf->specular_coefficient)),
            multiply_color(reflection_color, brdf->specular_coefficient));
      } else {
        double number_one =
            (n12 * vector_t::dot(normal, -myray.direction)) -
            sqrt(1 - ((n12 * n12) *
                      (1 - (vector_t::dot(normal, -myray.direction) *
                            vector_t::dot(normal, -myray.direction)))));
        vector_t t = (normal * number_one) - ((-myray.direction) * n12);
        t.normalize();
        ray_t refr_ray = ray_t(intersection, t);
        scene_hierarchy->RaySceneIntersection(refr_ray, refraction_color,
                                              recursion_depth + 1);
        color = add_color(
            multiply_color(add_color(multiply_color(reflection_color,
                                                    brdf->specular_coefficient),
                                     color),
                           (1 - brdf->transparency)),
            multiply_color(refraction_color, brdf->transparency));
      }
    }
    return true;
  }

  virtual bool RawIntersection(ray_t ray, point_t &intersection,
                               vector_t &normal, bool &inside) = 0;
  color_t MultiShade(vector_t normal, SceneHierarchy *scene_hierarchy,
                     point_t intersection, vector_t view_dir) {
    color_t result = {0, 0, 0};
    for (auto &&light : lights) {
      ray_t ray = ray_t(intersection, (light->position - intersection));
      color_t throwaway;
      if (scene_hierarchy->RaySceneIntersection(ray, throwaway)) {
        continue; // shadow
      }
      vector_t light_direction = (light->position - intersection).normalized();
      result = add_color(brdf->Shade(normal, light_direction, view_dir, light),
                         result);
    }
    result = add_color(result,
                       multiply_color(brdf->color, brdf->ambient_coefficient));
    return result;
  }
};

class Sphere : public Solids {
public:
  // todo: think about how to pass the lights vector around
  Sphere(std::shared_ptr<BRDF> brdf, std::shared_ptr<std::vector<Light>> lights)
      : Solids(brdf, lights) {}
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
  Plane(std::shared_ptr<BRDF> brdf, std::shared_ptr<std::vector<Light>> lights)
      : Solids(brdf, lights) {}

  bool RawIntersection(ray_t ray, point_t &intersection, vector_t &normal,
                       bool &inside) override {
    inside = false;
    normal = vector_t(0, 1, 0);
    if (std::abs(vector_t::dot(normal, ray.direction)) < 0.0000001) {
      intersection = point_t::zero();
      return false;
    }

    double t = -vector_t::dot(normal, ray.origin) /
               vector_t::dot(normal, ray.direction);

    intersection = ray.origin + ray.direction * t;

    if (t < 0.0000001) {
      return false;
    }

    return true;
  }
};

} // namespace raytracer
