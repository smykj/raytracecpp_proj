cmake_minimum_required(VERSION 3.30)

project(renderer)


#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


#add_executable(${PROJECT_NAME} main.cpp)


#target_link_libraries(${PROJECT_NAME} pugixml)
#add_subdirectory(src)
#add_subdirectory(lib)


find_package(pugixml CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)

add_subdirectory(src)
