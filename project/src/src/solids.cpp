#include "solids.hpp"
#include "scene_hierarchy.hpp"
namespace raytracer {

bool Solids::RayIntersection(ray_t ray, SceneHierarchy *scene_hierarchy,
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
    double n12 =
        (inside ? brdf->GetRefractionIndex() : 1 / brdf->GetRefractionIndex());

    if (brdf->specular_coefficient != 0) {
      scene_hierarchy->RaySceneIntersection(refl_ray, reflection_color,
                                            recursion_depth + 1);
    }

    if ((n12 > 1 && incidence_angle > brdf->critical_angle) ||
        brdf->transparency == 0) {
      color = color * (1 - brdf->specular_coefficient) +
              reflection_color * brdf->specular_coefficient;
    } else {
      double number_one =
          (n12 * vector_t::dot(normal, -myray.direction)) -
          sqrt(1 -
               ((n12 * n12) * (1 - (vector_t::dot(normal, -myray.direction) *
                                    vector_t::dot(normal, -myray.direction)))));
      vector_t t = (normal * number_one) - ((-myray.direction) * n12);
      t.normalize();
      ray_t refr_ray = ray_t(intersection, t);
      scene_hierarchy->RaySceneIntersection(refr_ray, refraction_color,
                                            recursion_depth + 1);
      color = reflection_color * brdf->specular_coefficient +
              color * (1 - brdf->transparency) +
              refraction_color * brdf->transparency;
    }
  }
  return true;
}

color_t Solids::MultiShade(vector_t normal, SceneHierarchy *scene_hierarchy,
                           point_t intersection, vector_t view_dir) {
  color_t result = {0, 0, 0};
  for (auto &&light : *lights) {
    ray_t ray = ray_t(intersection, (light.position - intersection));
    color_t throwaway;
    if (scene_hierarchy->RaySceneIntersection(ray, throwaway)) {
      continue; // shadow
    }
    vector_t light_direction = (light.position - intersection).normalized();
    result = brdf->Shade(normal, light_direction, view_dir, light) + result;
  }
  result = result + brdf->color * brdf->ambient_coefficient;
  return result;
}

} // namespace raytracer
