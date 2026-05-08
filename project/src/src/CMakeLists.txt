add_library(geometry geometry.cpp)
add_library(scene_hierarchy scene_hierarchy.cpp)
add_library(solids solids.cpp)
add_library(camera camera.cpp)
target_include_directories(camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(camera PRIVATE freeimage::FreeImage)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(
  ${PROJECT_NAME} PRIVATE scene_hierarchy solids geometry camera
                          pugixml::pugixml freeimage::FreeImage)
