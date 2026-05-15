add_library(geometry geometry.cpp)
add_library(scene_hierarchy scene_hierarchy.cpp)
add_library(solids solids.cpp)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(
  ${PROJECT_NAME} PRIVATE scene_hierarchy solids geometry pugixml::pugixml
                          freeimage::FreeImage)
