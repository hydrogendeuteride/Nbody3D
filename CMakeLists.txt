cmake_minimum_required(VERSION 3.25)
project(NBody3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-fopenmp")

include_directories(include)

add_executable(NBody3D main.cpp
        src/octree.cpp
        src/bhtree.cpp)