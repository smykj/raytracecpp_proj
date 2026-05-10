cmake_minimum_required(VERSION 3.30)

project(renderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(pugixml CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)

add_subdirectory(src)
