#include "scene_hierarchy.hpp"
#include "solids.hpp"
namespace raytracer {
bool LeafNode::RaySolidIntersection(ray_t ray, mat4d_t transformationToWorld,
                                    color_t &result_color, int recursionDepth,
                                    point_t &result_intersection) const {

  // std::cout << "BBBBBBBBBBB";
  // std::cout << "res inter " << result_intersection << std::endl;
  return solid->RayIntersection(ray, sh, transformationToWorld, result_color,
                                recursionDepth, result_intersection);
}

} // namespace raytracer
