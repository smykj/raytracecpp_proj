#pragma once
#include "geometry.hpp"
// #include "solids.hpp"
//  #include <array>
#include <cfloat>
#include <memory>
#include <vector>

namespace raytracer {
// typedef std::array<double, 3> color_t;
class SceneHierarchy;
class Solids;
class Node {
public:
  SceneHierarchy *sh;
  bool RaySolidIntersection(ray_t ray, mat4d_t transformationToWorld,
                            color_t &color, int recursionDepth,
                            point_t &intersection);
};

class InnerNode : public Node {
public:
  std::vector<std::shared_ptr<Node>> children;
  InnerNode(SceneHierarchy *sceneHierarchy, mat4d_t transformation)
      : transformation(transformation) {
    sh = sceneHierarchy; // in the original this was base.sh = sh; which i think
                         // makes no sense
    inv_transformation = transformation.inverted();
  }

  bool RaySolidIntersection(ray_t ray, mat4d_t transformationToWorld,
                            color_t &result_color, int recursionDepth,
                            point_t &result_intersection) {
    transformationToWorld *= inv_transformation;
    ray_t myray = ray.transformed(transformation);
    result_intersection = point_t::zero();
    result_color = {0, 0, 0};
    double nearest = DBL_MAX;
    bool result = false;

    for (auto &&child : children) {
      color_t color;
      point_t intersection;
      bool hit = child->RaySolidIntersection(
          myray, transformationToWorld, color, recursionDepth, intersection);
      double distance = (myray.origin - intersection).length();
      if (!hit || distance > nearest) {
        continue;
      }
      result = true;
      nearest = distance;
      result_intersection = intersection;
      result_color = color;
    }

    return result;
  }

private:
  mat4d_t transformation;
  mat4d_t inv_transformation;
};

class LeafNode : public Node {
  Solids *solid;

public:
  // todo: decide on how to pass solid
  LeafNode(SceneHierarchy *sceneHierarchy, Solids *solid) : solid(solid) {
    sh = sceneHierarchy;
  }

  bool RaySolidIntersection(ray_t ray, mat4d_t transformationToWorld,
                            color_t &result_color, int recursionDepth,
                            point_t &result_intersection);
};

class SceneHierarchy {
public:
  InnerNode root;
  int max_recursion_depth;
  color_t background_color;

  SceneHierarchy() : root(this, mat4d_t::identity()) {}

  bool RaySceneIntersection(ray_t ray, color_t &color,
                            int recursionDepth = -1) {
    point_t throwaway;
    bool result = root.RaySolidIntersection(ray, mat4d_t::identity(), color,
                                            recursionDepth, throwaway);
    if (!result) {
      color = background_color;
    }
    return result;
  }
};
} // namespace raytracer
