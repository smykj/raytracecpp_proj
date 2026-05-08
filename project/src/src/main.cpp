#include "camera.hpp"
#include "geometry.hpp"
#include "scene_hierarchy.hpp"
#include "solids.hpp"
#include <FreeImage.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include <pugixml.hpp>
#include <random>
#include <string>
#include <vector>

namespace raytracer {
// todo: setting refraction_index should also set critical_angle

// todo abstract function for shading

// struct PixelUnit {
//   static int antialias_dimension;
//   vec2i_t center_offset;
//   std::vector<ray_t> pixel;
//   SceneHierarchy *sh;
//
//   PixelUnit(SceneHierarchy *sh, point_t ofView, vec2i_t center_offset,
//             vector_t unit_up, vector_t unit_right, vector_t view_dir)
//       : center_offset(center_offset), sh(sh) {
//     vector_t antialias_unit_up = unit_up / antialias_dimension;
//     vector_t antialias_unit_right = unit_right / antialias_dimension;
//
//     std::uniform_real_distribution<double> distr(0.0, 1.0);
//     std::default_random_engine re;
//
//     for (size_t x = 0; x < antialias_dimension; ++x) {
//       for (size_t y = 0; y < antialias_dimension; ++y) {
//         pixel.push_back(ray_t(ofView, (view_dir + (antialias_unit_right * x)
//         +
//                                        (antialias_unit_right * distr(re)) +
//                                        (antialias_unit_up * y) +
//                                        (antialias_unit_up * distr(re)))));
//       }
//     }
//   }
//   color_t GetColor() {
//     color_t res = {0, 0, 0};
//     for (auto &&ray : pixel) {
//       color_t result;
//       bool hit = sh->RaySceneIntersection(ray, result, 0);
//       if (!hit) {
//         result = sh->background_color;
//       }
//     }
//     res[0] /= (double)pixel.size();
//     res[1] /= (double)pixel.size();
//     res[2] /= (double)pixel.size();
//     return res;
//   }
// };
//
// struct ComputeUnit {
//   const static int compute_dimension = 16;
//   vec2i_t center_offset;
//   std::vector<PixelUnit> region;
//
//   ComputeUnit(const std::shared_ptr<FIBITMAP> &image, SceneHierarchy *sh,
//               point_t of_view, vec2i_t center_offset, vector_t unit_up,
//               vector_t unit_right, vector_t view_dir)
//       : center_offset(center_offset) {
//     auto height = FreeImage_GetHeight(image.get());
//     auto width = FreeImage_GetWidth(image.get());
//     region.reserve(static_cast<size_t>(compute_dimension) *
//     compute_dimension); for (size_t x = 0; x < compute_dimension; ++x) {
//       for (size_t y = 0; y < compute_dimension; ++y) {
//         vec2i_t center_offset_pixel = {static_cast<int>(center_offset.x + x),
//                                        static_cast<int>(center_offset.y +
//                                        y)};
//         if (center_offset_pixel.x > width || center_offset_pixel.y > height)
//         {
//           continue;
//         }
//
//         vector_t pixel_view_dir = view_dir +
//                                   (unit_right * (center_offset.x + x)) +
//                                   (unit_up * (center_offset.y + y));
//         region.push_back(PixelUnit(sh, of_view, center_offset_pixel, unit_up,
//                                    unit_right, pixel_view_dir));
//       }
//     }
//   }
//
//   void FillRegion(const std::shared_ptr<FIBITMAP> &img) {
//     auto height = FreeImage_GetHeight(img.get());
//     auto width = FreeImage_GetWidth(img.get());
//     for (auto &&pixel : region) {
//       color_t result = pixel.GetColor();
//       // todo: convert doubles to char and assign
//       RGBQUAD colour{
//           .rgbBlue = 100, .rgbGreen = 100, .rgbRed = 100, .rgbReserved = 0};
//
//       FreeImage_SetPixelColor(img.get(), pixel.center_offset.x + (width / 2),
//                               height - pixel.center_offset.y - (height / 2),
//                               &colour);
//     }
//   }
// };

} // namespace raytracer

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

  // std::string config_file_name = argv[1];
  std::vector<std::shared_ptr<BRDF>> brdfs;
  std::vector<Light> lights;
  Camera cam = Camera();
  SceneHierarchy scene_hierarchy = SceneHierarchy();
  scene_hierarchy.max_recursion_depth = 10;

  pugi::xml_document xml_doc;
  pugi::xml_parse_result res = xml_doc.load_file(argv[1]);

  if (!res) {
    std::cout << "Failed to parse xml file.\n";
    return -1;
  }

  // vector_t v{1, 2, 3};
  // point_t p{1, 2, 3};
  //// the syntax for struct initialization is insane, who designed this?
  // mat4d_t m{{{{{2, 2, 2, 2}}, {{1, 1, 1, 1}}, {{0, 0, 0, 0}}, {{3, 3, 3,
  // 3}}}}};

  // for (int i = 0; i < 4; ++i) {

  //  for (int k = 0; k < 4; ++k) {
  //    std::cout << m[i][k];
  //  }
  //}
  // std::cout << "\n";
  // vector_t tr = v * m;
  // point_t trp = p * m;

  // std::cout << "point: " << trp.x << " " << trp.y << " " << trp.z << '\n';
  // std::cout << "vec: " << tr.x << " " << tr.y << " " << tr.z << '\n';

  // vector_t vec;
  // vec.x = 3;
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file("config2.xml");
  if (!result) {
    std::cout << "Failed to parse xml file.\n";
    return -1;
  }

  pugi::xml_node config = doc.child("configuration");
  pugi::xml_node image_conf = config.child("image_conf");
  int width = std::stoi(image_conf.attribute("width").value());
  int height = std::stoi(image_conf.attribute("height").value());
  const char *filename = image_conf.attribute("filename").value();
  scene_hierarchy.max_recursion_depth =
      std::stoi(image_conf.attribute("recursion_depth").value());
  PixelUnit::antialias_dimension =
      std::stoi(image_conf.attribute("antialiasing").value());

  auto phongs = doc.children("phong_material");
  for (auto &&phong : phongs) {
    std::shared_ptr<Phong> pm = std::make_shared<Phong>(Phong());
    pm->ambient_coefficient = std::stod(phong.attribute("amb_coef").value());
    pm->diffuse_coefficient = std::stod(phong.attribute("dif_coef").value());
    pm->specular_coefficient = std::stod(phong.attribute("spec_coef").value());
    pm->specular_exponent = std::stoi(phong.attribute("spec_expo").value());
    pm->color = xmlstrtovec3<color_t>(phong.attribute("color").value());
    int id = std::stoi(phong.attribute("id").value());
    pm->transparency = std::stod(phong.attribute("transparency").value());
    pm->SetRefractionIndex(std::stod(phong.attribute("refr_index").value()));
    brdfs.insert(std::next(brdfs.begin(), id),
                 pm); // this insert will probably fail if the vector isnt long
                      // enough (which it almost definitely isnt)
  }

  auto orens = doc.children("orennayar_material");
  for (auto &&oren : orens) {
    std::shared_ptr<OrenNayar> on = std::make_shared<OrenNayar>(OrenNayar());
    on->ambient_coefficient = std::stod(oren.attribute("amb_coef").value());
    on->roughness = std::stod(oren.attribute("roughness").value());
    on->specular_coefficient = std::stod(oren.attribute("spec_coef").value());
    on->color = xmlstrtovec3<color_t>(oren.attribute("color").value());
    int id = std::stoi(oren.attribute("id").value());
    on->transparency = std::stod(oren.attribute("transparency").value());
    on->SetRefractionIndex(std::stod(oren.attribute("refr_index").value()));
    on->diffuse_coefficient = std::stod(oren.attribute("dif_coef").value());
    on->rms = std::stod(oren.attribute("rms").value());
    brdfs.insert(std::next(brdfs.begin(), id),
                 on); // this insert will probably fail if the vector isnt long
                      // enough (which it almost definitely isnt)
  }

  auto lts = doc.children("light");
  for (auto &&lt : lts) {
    Light light = Light();
    light.position = xmlstrtovec3<point_t>(lt.attribute("position").value());
    light.color = xmlstrtovec3<color_t>(lt.attribute("color").value());
    light.intensity = xmlstrtovec3<color_t>(lt.attribute("intensity").value());
    lights.push_back(light);
  }

  auto spheres = doc.children("sphere");
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

  auto planes = doc.children("plane");
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

  auto camera = doc.child("camera");
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
  // std::vector<temp_BRDF> brdfs;
  // std::vector<temp_light> lights;
  // temp_camera camera;
  // temp_scene_hierarchy scene_hierarchy;
  // temp_pixel_unit pixel_unit;

  // pugi::xml_node config = doc.child("configuration");

  // int width = image_conf.attribute("width").as_int();
  // int height = image_conf.attribute("height").as_int();
  // std::string file_name = image_conf.attribute("filename").as_string();
  // scene_hierarchy.max_recursion_depth =
  // image_conf.attribute("recursion_depth").as_int();
  // pixel_unit.antialias_dimension =
  // image_conf.attribute("antialiasing").as_int();

  // pugi::xml_node camera_conf = config.child("camera");

  // camera.direction = camera_conf.attribute("direction")

  // std::cout << width << '\n';
  // for (pugi::xml_node tool : doc.child("configuration").children()) {

  // std::cout << tool.name() << '\n';
  // }
  // constexpr unsigned WIDTH = 256;
  // constexpr unsigned HEIGHT = 256;

  // FreeImage_Initialise();

  // FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, 24);
  // if (bitmap == nullptr) {
  //   std::cerr << "allocate failed\n";
  //   FreeImage_DeInitialise();
  //   return 1;
  // }

  // for (unsigned y = 0; y < HEIGHT; ++y) {
  //   for (unsigned x = 0; x < WIDTH; ++x) {
  //     RGBQUAD colour{
  //         .rgbBlue = 100, .rgbGreen = 100, .rgbRed = 100, .rgbReserved = 0};

  //    FreeImage_SetPixelColor(bitmap, x, (HEIGHT - 1) - y, &colour);
  //  }
  //}

  const char *outputPath = filename;
  if (FreeImage_Save(FIF_PNG, image.get(), outputPath, PNG_DEFAULT) != 0) {
    std::cout << "saved: " << outputPath << "\n";
  } else {
    std::cerr << "save failed\n";
  }

  // FreeImage_Unload(bitmap);
  // FreeImage_DeInitialise();

  return 0;
}
