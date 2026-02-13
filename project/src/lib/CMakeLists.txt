add_library(pugixml pugixml.cpp)

target_include_directories(pugixml
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})
