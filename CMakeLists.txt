cmake_minimum_required(VERSION 3.20)
project(BouncePP)

set(CMAKE_CXX_STANDARD 20)

find_package(EnTT CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

add_executable(
        BouncePP src/main.cpp
        src/vendor/stb/stb_image.h
        src/ecs/components/components.hpp
        src/ecs/factories/factory.h
        src/ecs/factories/factory.cpp
        src/ecs/systems/physicsSystem.h
        src/ecs/systems/physicsSystem.cpp
        src/ecs/systems/InputSystem.h
        src/ecs/systems/InputSystem.cpp
        src/ecs/systems/GameLogic.cpp
        src/ecs/systems/GameLogic.h
)

target_link_libraries(BouncePP PRIVATE EnTT::EnTT SDL3::SDL3 box2d::box2d)
