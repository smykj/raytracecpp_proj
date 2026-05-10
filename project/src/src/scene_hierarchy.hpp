#pragma once
#include "geometry.hpp"
#include "solids.hpp"
//  #include <array>
#include <cfloat>
#include <iostream>
#include <memory>
#include <vector>

namespace raytracer {
// typedef std::array<double, 3> color_t;
class SceneHierarchy;
class Solids;
class Node {
public:
  SceneHierarchy *sh;
  // virtual ~Node() noexcept = default;
  virtual bool RaySolidIntersection(ray_t ray, mat4d_t transformationToWorld,
                                    color_t &color, int recursionDepth,
                                    point_t &intersection) const = 0;
};

class InnerNode : public Node {
public:
  // ~InnerNode() noexcept override = default;
  std::vector<std::shared_ptr<Node>> children;
  InnerNode(SceneHierarchy *sceneHierarchy, mat4d_t transformation)
      : transformation(transformation) {
    sh = sceneHierarchy; // in the original this was base.sh = sh; which i think
                         // makes no sense
    inv_transformation = transformation.inverted();
  }

  virtual bool
  RaySolidIntersection(ray_t ray, mat4d_t transformationToWorld,
                       color_t &result_color, int recursionDepth,
                       point_t &result_intersection) const override {
    // std::cout << "rr" << ray.origin << std::endl;
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
  std::unique_ptr<Solids> solid;
  // Solids *solid;

public:
  // ~LeafNode() noexcept override = default;
  LeafNode() = default;
  LeafNode(SceneHierarchy *sceneHierarchy, std::unique_ptr<Solids> solid)
      : solid(std::move(solid)) {
    sh = sceneHierarchy;
  }

  virtual bool
  RaySolidIntersection(ray_t ray, mat4d_t transformationToWorld,
                       color_t &result_color, int recursionDepth,
                       point_t &result_intersection) const override;
};

class SceneHierarchy {
public:
  InnerNode root;
  int max_recursion_depth;
  color_t background_color;

  SceneHierarchy() : root(this, mat4d_t::identity()) {}

  bool RaySceneIntersection(ray_t ray, color_t &color,
                            int recursionDepth = -1) const {
    // std::cout << ray.origin << std::endl;
    point_t throwaway;
    // std::cout << "ROOT CHILDREN: " << root.children.size() << std::endl;
    bool result = root.RaySolidIntersection(ray, mat4d_t::identity(), color,
                                            recursionDepth, throwaway);

    // std::cout << "INSIDE RAY SCENE INTERSECTION 2" << std::endl;
    if (!result) {
      // std::cout << "NO HIT " << ray.origin << " " << ray.direction <<
      // std::endl;
      color = background_color;
    }
    return result;
  }
};
} // namespace raytracer
