#include "geometry.hpp"
#include <FreeImage.h>
#include <iostream>
#include <pugixml.hpp>
#include <vector>

struct temp_BRDF {
  double transparency;
  double refraction_index;
  double critical_angle;
  double specular_coefficient;
  vector color;
  double ambient_coefficient;
  double diffuse_coefficient;
  // todo: setting refraction_index should also set critical_angle

  // todo abstract function for shading
};

struct temp_light {
  point position; // should be "point" not temp_vec3d
  vector color;
  vector intensity;
  // and constructor
};

struct temp_camera {
  point position; // should be point
  vector direction;
  vector up;
  vec2i screen_dimensions;
  float field_of_view;
};

struct temp_leaf_node {};

struct temp_scene_hierarchy {
  temp_leaf_node root;
  int max_recursion_depth;
  vector background_color;
};

struct temp_pixel_unit {
  int antialias_dimension; // should be ""static"" - same for all instances
  vec2i center_offset;
  // list of rays
  temp_scene_hierarchy sh;
  // random number generator
};

int main() {
  vector v{1, 2, 3};
  point p{1, 2, 3};
  mat4d m{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

  vector tr = v * m;
  point trp = p * m;

  std::cout << "point: " << trp.x << " " << trp.y << " " << trp.z << '\n';
  std::cout << "vec: " << tr.x << " " << tr.y << " " << tr.z << '\n';

  vector vec;
  vec.x = 3;
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file("config2.xml");
  if (!result) {
    std::cout << "Failed to parse xml file.\n";
    return -1;
  }

  std::vector<temp_BRDF> brdfs;
  std::vector<temp_light> lights;
  temp_camera camera;
  temp_scene_hierarchy scene_hierarchy;
  temp_pixel_unit pixel_unit;

  pugi::xml_node config = doc.child("configuration");

  pugi::xml_node image_conf = config.child("image_conf");
  int width = image_conf.attribute("width").as_int();
  int height = image_conf.attribute("height").as_int();
  std::string file_name = image_conf.attribute("filename").as_string();
  scene_hierarchy.max_recursion_depth =
      image_conf.attribute("recursion_depth").as_int();
  pixel_unit.antialias_dimension =
      image_conf.attribute("antialiasing").as_int();

  pugi::xml_node camera_conf = config.child("camera");

  // camera.direction = camera_conf.attribute("direction")

  std::cout << width << '\n';
  for (pugi::xml_node tool : doc.child("configuration").children()) {

    std::cout << tool.name() << '\n';
  }
  constexpr unsigned WIDTH = 256;
  constexpr unsigned HEIGHT = 256;

  FreeImage_Initialise();

  FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, 24);
  if (!bitmap) {
    std::cerr << "allocate failed\n";
    FreeImage_DeInitialise();
    return 1;
  }

  for (unsigned y = 0; y < HEIGHT; ++y) {
    for (unsigned x = 0; x < WIDTH; ++x) {
      RGBQUAD colour{
          .rgbBlue = 100, .rgbGreen = 100, .rgbRed = 100, .rgbReserved = 0};

      FreeImage_SetPixelColor(bitmap, x, (HEIGHT - 1) - y, &colour);
    }
  }

  const char *outputPath = "picture.png";
  if (FreeImage_Save(FIF_PNG, bitmap, outputPath, PNG_DEFAULT)) {
    std::cout << "saved: " << outputPath << "\n";
  } else {
    std::cerr << "save failed\n";
  }

  FreeImage_Unload(bitmap);
  FreeImage_DeInitialise();

  return 0;
}
