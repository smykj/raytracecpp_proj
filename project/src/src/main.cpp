
#include "camera.hpp"
#include "geometry.hpp"
#include "scene_hierarchy.hpp"
#include "solids.hpp"
#include <FreeImage.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include <pugixml.hpp>
#include <string>
#include <vector>
template <typename T> T xmlstrtovec3(const std::string &input) {
  std::array<double, 3> res;
  std::stringstream ss(input);
  std::string tok;
  std::getline(ss, tok, ' ');
  res[0] = std::stod(tok);
  std::getline(ss, tok, ' ');
  res[1] = std::stod(tok);
  std::getline(ss, tok, ' ');
  res[2] = std::stod(tok);
  return {res[0], res[1], res[2]};
}

using namespace raytracer;
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "wrong amount of arguments, provide name of config file\n";
  }
  std::vector<std::shared_ptr<BRDF>> brdfs;
  std::vector<Light> lights;
  Camera cam = Camera();
  SceneHierarchy scene_hierarchy = SceneHierarchy();
  scene_hierarchy.max_recursion_depth = 2;

  pugi::xml_document xml_doc;
  pugi::xml_parse_result res = xml_doc.load_file(argv[1]);

  if (!res) {
    std::cout << "Failed to parse xml file.\n";
    return -1;
  }

  pugi::xml_node config = xml_doc.child("configuration");
  pugi::xml_node image_conf = config.child("image_conf");
  int width = std::stoi(image_conf.attribute("width").value());
  int height = std::stoi(image_conf.attribute("height").value());
  const char *filename = image_conf.attribute("filename").value();
  scene_hierarchy.max_recursion_depth =
      std::stoi(image_conf.attribute("recursion_depth").value());
  PixelUnit::antialias_dimension =
      std::stoi(image_conf.attribute("antialiasing").value());

  auto phongs = config.children("phong_material");
  for (auto &&phong : phongs) {
    std::shared_ptr<Phong> pm = std::make_shared<Phong>(Phong());
    pm->ambient_coefficient = std::stod(phong.attribute("amb_coef").value());
    pm->diffuse_coefficient = std::stod(phong.attribute("dif_coef").value());
    pm->specular_coefficient = std::stod(phong.attribute("spec_coef").value());
    pm->specular_exponent = std::stoi(phong.attribute("spec_expo").value());
    pm->color = xmlstrtovec3<color_t>(phong.attribute("color").value());
    size_t id = std::stoi(phong.attribute("id").value());
    pm->transparency = std::stod(phong.attribute("transparency").value());
    pm->SetRefractionIndex(std::stod(phong.attribute("refr_index").value()));
    brdfs.resize(std::max(brdfs.size(), id) + 1);
    brdfs[id] = pm;
  }

  auto orens = config.children("orennayar_material");
  for (auto &&oren : orens) {
    std::shared_ptr<OrenNayar> on = std::make_shared<OrenNayar>(OrenNayar());
    on->ambient_coefficient = std::stod(oren.attribute("amb_coef").value());
    on->roughness = std::stod(oren.attribute("roughness").value());
    on->specular_coefficient = std::stod(oren.attribute("spec_coef").value());
    on->color = xmlstrtovec3<color_t>(oren.attribute("color").value());
    size_t id = std::stoi(oren.attribute("id").value());
    on->transparency = std::stod(oren.attribute("transparency").value());
    on->SetRefractionIndex(std::stod(oren.attribute("refr_index").value()));
    on->diffuse_coefficient = std::stod(oren.attribute("dif_coef").value());
    on->rms = std::stod(oren.attribute("rms").value());
    brdfs.resize(std::max(brdfs.size(), id) + 1);
    brdfs[id] = on;
  }

  auto lts = config.children("light");
  for (auto &&lt : lts) {
    Light light = Light();
    light.position = xmlstrtovec3<point_t>(lt.attribute("position").value());
    light.color = xmlstrtovec3<color_t>(lt.attribute("color").value());
    light.intensity = xmlstrtovec3<color_t>(lt.attribute("intensity").value());
    lights.push_back(light);
  }

  auto spheres = config.children("sphere");
  for (auto &&sphere : spheres) {
    vector_t centre =
        xmlstrtovec3<vector_t>(sphere.attribute("centre").value());
    double radius = std::stod(sphere.attribute("radius").value());
    int material_id = std::stoi(sphere.attribute("material_id").value());

    mat4d_t c = mat4d_t::scale(radius);
    mat4d_t e = mat4d_t::create_translation(centre);
    mat4d_t m = c * e;
    std::shared_ptr<InnerNode> tr =
        std::make_shared<InnerNode>(InnerNode(&scene_hierarchy, m.inverted()));
    std::shared_ptr<LeafNode> lf = std::make_shared<LeafNode>(
        LeafNode(&scene_hierarchy,
                 std::make_unique<Sphere>(Sphere(brdfs[material_id], lights))));
    tr->children.push_back(lf);
    scene_hierarchy.root.children.push_back(tr);
  }

  auto planes = config.children("plane");

  for (auto &&plane : planes) {
    vector_t normal = xmlstrtovec3<vector_t>(plane.attribute("normal").value());
    double d = std::stod(plane.attribute("d").value());
    int material_id = std::stoi(plane.attribute("material_id").value());
    mat4d_t a;
    if (normal == vector_t{0, 1, 0}) {
      a = mat4d_t::identity();
    } else {
      a = mat4d_t::axis_angle(
          vector_t::cross(vector_t({0, 1, 0}), normal).normalized(),
          vector_t::angle(vector_t{0, 1, 0}, normal));
    }
    mat4d_t b = mat4d_t::create_translation(normal * -d);
    std::shared_ptr<InnerNode> tr =
        std::make_shared<InnerNode>(&scene_hierarchy, b * a);
    std::shared_ptr<LeafNode> lf = std::make_shared<LeafNode>(
        LeafNode(&scene_hierarchy,
                 std::make_unique<Plane>(Plane(brdfs[material_id], lights))));
    tr->children.push_back(lf);
    scene_hierarchy.root.children.push_back(tr);
  }

  auto camera = config.child("camera");
  point_t position =
      xmlstrtovec3<point_t>(camera.attribute("position").value());
  vector_t direction =
      xmlstrtovec3<vector_t>(camera.attribute("direction").value());
  double fov = std::stod(camera.attribute("fov").value());
  vector_t up = xmlstrtovec3<vector_t>(camera.attribute("up").value());
  color_t background_color =
      xmlstrtovec3<color_t>(camera.attribute("background_color").value());
  cam = Camera(position, direction, vec2i_t(width, height), fov, up);
  scene_hierarchy.background_color = background_color;

  auto image = cam.Raycast(scene_hierarchy);

  const char *outputPath = filename;
  if (FreeImage_Save(FIF_PNG, image.get(), filename, PNG_DEFAULT) != 0) {
    std::cout << "saved: " << outputPath << "\n";
  } else {
    std::cerr << "save failed\n";
  }

  return 0;
}
