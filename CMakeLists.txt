cmake_minimum_required(VERSION 3.20)
project(BouncePP)

set(CMAKE_CXX_STANDARD 20)

find_package(EnTT CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

add_executable(BouncePP main.cpp
        vendor/stb/stb_image.h)

target_link_libraries(BouncePP PRIVATE EnTT::EnTT SDL3::SDL3 box2d::box2d)
