# add_library(mymathlib mymathlib.cpp)
add_library(geometry geometry.cpp)
add_library(scene_hierarchy scene_hierarchy.cpp)
add_library(solids solids.cpp)
# target_include_directories(mymathlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(geometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(
  ${PROJECT_NAME} PRIVATE scene_hierarchy solids geometry pugixml::pugixml
                          freeimage::FreeImage)
