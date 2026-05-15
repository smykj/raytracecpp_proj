#pragma once
#include "geometry.hpp"
#include "scene_hierarchy.hpp"
#include <FreeImage.h>
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <thread>
namespace raytracer {

struct PixelUnit {
  inline static int antialias_dimension = 0;
  vec2i_t center_offset;
  std::vector<ray_t> pixel;
  const SceneHierarchy *sh;

  PixelUnit(const SceneHierarchy &sh, point_t ofView, vec2i_t center_offset,
            vector_t unit_up, vector_t unit_right, vector_t view_dir)
      : center_offset(center_offset), sh(&sh) {
    vector_t antialias_unit_up = unit_up / antialias_dimension;
    vector_t antialias_unit_right = unit_right / antialias_dimension;

    std::uniform_real_distribution<double> distr(0.0, 1.0);
    std::default_random_engine re;

    for (int x = 0; x < antialias_dimension; ++x) {
      for (int y = 0; y < antialias_dimension; ++y) {
        vector_t dir =
            (view_dir + (antialias_unit_right * x) +
             (antialias_unit_right * distr(re)) + (antialias_unit_up * y) +
             (antialias_unit_up * distr(re)));
        pixel.push_back(ray_t(ofView, dir.normalized()));
      }
    }
  }

  color_t GetColor() {
    color_t res = {0, 0, 0};
    for (auto &&ray : pixel) {
      color_t result;
      bool hit = sh->RaySceneIntersection(ray, result, 0);
      if (!hit) {
        result = sh->background_color;
      }
      res += result;
    }
    res[0] /= (double)pixel.size();
    res[1] /= (double)pixel.size();
    res[2] /= (double)pixel.size();
    return res;
  }
};

double counter = 0;
struct ComputeUnit {
  const static int compute_dimension = 16;
  vec2i_t center_offset;
  std::vector<PixelUnit> region;
  ComputeUnit() = default;
  ComputeUnit(const std::shared_ptr<FIBITMAP> &image, const SceneHierarchy &sh,
              point_t of_view, vec2i_t center_offset, vector_t unit_up,
              vector_t unit_right, vector_t view_dir)
      : center_offset(center_offset) {
    auto height = (int)FreeImage_GetHeight(image.get());
    auto width = (int)FreeImage_GetWidth(image.get());
    region.reserve(static_cast<size_t>(compute_dimension) * compute_dimension);
    for (int x = 0; x < compute_dimension; ++x) {
      for (int y = 0; y < compute_dimension; ++y) {
        vec2i_t center_offset_pixel = {(center_offset.x + x),
                                       (center_offset.y + y)};
        if (center_offset_pixel.x > width || center_offset_pixel.y > height) {
          continue;
        }
        vector_t pixel_view_dir = view_dir +
                                  (unit_right * (center_offset.x + x)) +
                                  (unit_up * (center_offset.y + y));
        region.push_back(PixelUnit(sh, of_view, center_offset_pixel, unit_up,
                                   unit_right, pixel_view_dir));
      }
    }
  }

  void FillRegion(const std::shared_ptr<FIBITMAP> &img) {
    auto height = FreeImage_GetHeight(img.get());
    auto width = FreeImage_GetWidth(img.get());
    for (auto &&pixel : region) {
      color_t result = pixel.GetColor();
      if (result[0] > 1 || result[2] > 1 || result[1] > 1) {
        ++counter;
      }
      uint8_t red = std::floor(std::min(result[0], 1.0) * 255);
      uint8_t green = std::floor(std::min(result[1], 1.0) * 255);
      uint8_t blue = std::floor(std::min(result[2], 1.0) * 255);
      RGBQUAD colour{
          .rgbBlue = blue, .rgbGreen = green, .rgbRed = red, .rgbReserved = 0};
      FreeImage_SetPixelColor(img.get(), pixel.center_offset.x + (width / 2),
                              pixel.center_offset.y + (height / 2), &colour);
    }
  }
};

struct Camera {
  point_t position = point_t::zero();
  vector_t direction = vector_t::zero();
  vector_t up = vector_t::zero();
  vec2i_t screen_dimensions;
  double fov;

  Camera() = default;
  Camera(point_t position, vector_t direction, vec2i_t screen_dimensions,
         double fov, vector_t up)
      : position(position), direction(direction), up(up),
        screen_dimensions(screen_dimensions), fov(fov * (3.14159 / 180)) {}

  std::shared_ptr<FIBITMAP> Raycast(
      const SceneHierarchy &scene_hierarchy) { // todo return some form of image
    mat4d_t rotation = mat4d_t::axis_angle(up, fov / 2);
    vector_t dir_rotated_l = direction * rotation;
    dir_rotated_l.normalize();
    vector_t dir_rotated_r = direction * rotation.inverted();
    dir_rotated_r.normalize();
    vector_t deltav = dir_rotated_r - dir_rotated_l;
    double delta = deltav.length() / screen_dimensions.x;
    FreeImage_Initialise();

    auto bitmap = std::shared_ptr<FIBITMAP>(
        FreeImage_Allocate(screen_dimensions.x, screen_dimensions.y, 24),
        FreeImage_Unload);

    if (!bitmap) {
      std::cerr << "ERROR: Image allocation failed.\n";
      FreeImage_DeInitialise();
      return nullptr;
    }

    std::queue<ComputeUnit> cu;
    vector_t unit_up = up.normalized() * delta;
    vector_t unit_right = deltav.normalized() * delta;

    for (int x = -screen_dimensions.x / 2; x < screen_dimensions.x / 2;
         x += ComputeUnit::compute_dimension) {
      for (int y = -screen_dimensions.y / 2; y < screen_dimensions.y / 2;
           y += ComputeUnit::compute_dimension) {
        cu.push(ComputeUnit(bitmap, scene_hierarchy, position, vec2i_t(x, y),
                            unit_up, unit_right, direction));
      }
    }

    size_t all = cu.size();

    std::vector<std::jthread> pool;
    size_t poolsize = std::thread::hardware_concurrency();
    pool.reserve(poolsize);
    std::mutex popmtx;
    auto parapop = [&]() -> std::optional<ComputeUnit> {
      popmtx.lock();
      if (cu.size() == 0) {
        popmtx.unlock();
        return {};
      }
      ComputeUnit item = cu.front();
      cu.pop();
      std::cout << "\r"
                << std::ceil(100 - ((float)cu.size() * 100 / (float)all))
                << "%       ";
      popmtx.unlock();
      return item;
    };

    for (size_t i = 0; i < poolsize; ++i) {
      pool.emplace_back([=]() {
        while (auto unit = parapop()) {
          unit.value().FillRegion(bitmap);
        }
      });
    }
    return bitmap;
  }
};

} // namespace raytracer
