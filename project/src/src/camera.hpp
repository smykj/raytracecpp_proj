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
        // std::cout << "povv" << view_dir << std::endl;
        vector_t dir =
            (view_dir + (antialias_unit_right * x) +
             (antialias_unit_right * distr(re)) + (antialias_unit_up * y) +
             (antialias_unit_up * distr(re)));
        pixel.push_back(ray_t(ofView, dir.normalized()));
      }
    }

#ifdef DEBUG
    std::cout << "generated ray origin: " << pixel[0].origin
              << " ray direction: " << pixel[0].direction << std::endl;
    std::cout << "offset x: " << center_offset.x << " y: " << center_offset.y
              << std::endl;
#endif
  }
  color_t GetColor() {
    color_t res = {0, 0, 0};
    for (auto &&ray : pixel) {
      // std::cout << "gr" << ray.direction << std::endl;
      color_t result;

      // std::cout << "INSIDE GET COLOR PIXEL LOOP" << std::endl;
      bool hit = sh->RaySceneIntersection(ray, result, 0);
      // std::cout << "INSIDE GET COLOR PIXEL LOOP 2" << std::endl;
      // if (hit)
      // std::cout << "AAAAAAAAAAAAAAAA";
      if (!hit) {

#ifdef DEBUG
        std::cout << "Top level background hit\n";
#endif
        result = sh->background_color;
      }
      res += result;
    }
    res[0] /= (double)pixel.size();
    res[1] /= (double)pixel.size();
    res[2] /= (double)pixel.size();
    // std::cout << res[0];
    return res;
  }
};

double counter = 0;
struct ComputeUnit {
  const static int compute_dimension = 16; // todo set to 16
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

        // std::cout << "ovv" << view_dir << std::endl;
        // std::cout << "ur" << unit_right << std::endl;
        // std::cout << "up" << unit_up << std::endl;
        // std::cout << "co" << center_offset.x << center_offset.y << std::endl;
        vector_t pixel_view_dir = view_dir +
                                  (unit_right * (center_offset.x + x)) +
                                  (unit_up * (center_offset.y + y));

        // std::cout << "povv" << pixel_view_dir << std::endl;
        region.push_back(PixelUnit(sh, of_view, center_offset_pixel, unit_up,
                                   unit_right, pixel_view_dir));
      }
    }
  }

  void FillRegion(const std::shared_ptr<FIBITMAP> &img) {
    // std::cout << "INSIDE FILL REGION" << std::endl;
    // std::cout << "REGION SIZE: " << region.size() << std::endl;
    auto height = FreeImage_GetHeight(img.get());
    auto width = FreeImage_GetWidth(img.get());
    for (auto &&pixel : region) {
      // std::cout << "INSIDE PIXEL LOOP" << std::endl;

#ifdef DEBUG
      std::cout << "ray: " << pixel.pixel[0].origin << " "
                << pixel.pixel[0].direction << std::endl;
#endif
      color_t result = pixel.GetColor();
      // std::cout << "INSIDE PIXEL LOOP 2" << std::endl;
      // std::cout << result[0];
      // counter = std::max(counter, result[0]);
      // counter = std::max(counter, result[1]);
      // counter = std::max(counter, result[2]);
      if (result[0] > 1 || result[2] > 1 || result[1] > 1) {
        ++counter;
      }
      uint8_t red = std::floor(std::min(result[0], 1.0) * 255);
      uint8_t green = std::floor(std::min(result[1], 1.0) * 255);
      uint8_t blue = std::floor(std::min(result[2], 1.0) * 255);
#ifdef DEBUG
      std::cout << "RESULT " << result[0] << " " << result[1] << " "
                << result[2] << "\n"
                << std::endl;
#endif
      // todo: convert doubles to char and assign
      // std::cout << "x->" << pixel.center_offset.x << " y->"
      // << pixel.center_offset.y << std::endl;
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
    // std::cout << "dir" << direction << std::endl;
    // std::cout << "rotation" << rotation.inverted() << std::endl;
    // std::cout << "dirl" << dir_rotated_l << std::endl;
    double delta = deltav.length() / screen_dimensions.x;
    FreeImage_Initialise();

    // note to self on how this works:
    // freeimage library is made in C, so the expected way to
    // work with it is to manage your own memory. however, the c++ smart
    // pointers take custom deallocators for the data that it points to, and
    // either through sheer luck or through some amazingly clarvoyant design
    // decisions, the function signatures for the custom deallocator and the
    // FreeImage_Unload function match exactly
    auto bitmap = std::shared_ptr<FIBITMAP>(
        FreeImage_Allocate(screen_dimensions.x, screen_dimensions.y, 24),
        FreeImage_Unload);

    if (!bitmap) {
      std::cerr << "ERROR: Image allocation failed.\n";
      FreeImage_DeInitialise();
      return nullptr;
    }

    std::queue<ComputeUnit> cu;
    // cu.reserve(
    // (screen_dimensions.x * screen_dimensions.y) /
    // (ComputeUnit::compute_dimension * ComputeUnit::compute_dimension));

    // std::cout << "dv" << deltav << std::endl;
    // std::cout << "delt" << delta << std::endl;
    vector_t unit_up = up.normalized() * delta;
    vector_t unit_right = deltav.normalized() * delta;

    for (int x = -screen_dimensions.x / 2; x < screen_dimensions.x / 2;
         x += ComputeUnit::compute_dimension) {
      for (int y = -screen_dimensions.y / 2; y < screen_dimensions.y / 2;
           y += ComputeUnit::compute_dimension) {
        // std::cout << "ur" << unit_right << std::endl;
        // std::cout << "up" << unit_up << std::endl;
        cu.push(ComputeUnit(bitmap, scene_hierarchy, position, vec2i_t(x, y),
                            unit_up, unit_right, direction));
        // cu.push_back(ComputeUnit{.fi = bitmap.get(),
        //                          .scene_hierarchy = &scene_hierarchy,
        //                          .of_view = position,
        //                          .center_offset = vec2i_t(x, y),
        //                          .unit_up = unit_up,
        //                          .unit_right = unit_right,
        //                          .view_dir = direction});
      }
    }

    // size_t mod = cu.size() / 100;
    size_t counter = 0;

    // todo: parallelism
    // std::for_each(std::execution::par_unseq,cu.begin(), cu.end(),
    // [&](ComputeUnit &unit) { unit.FillRegion(bitmap);
    // });

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
      std::cout << cu.size() << "\n";
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

    std::cout << "COMPUTE UNITS: " << cu.size() << std::endl;
    // for (auto &&unit : cu) {
#ifndef DEBUG
    // std::cout << "\r" << counter * 100 / cu.size() << "%    ";
#endif
    // unit.FillRegion(bitmap);
    // ++counter;
    // }
    std::cout << "\n";

    return bitmap;
  }
};

} // namespace raytracer
