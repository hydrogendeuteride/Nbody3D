cmake_minimum_required(VERSION 3.25)
project(NBody3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-fopenmp")

include_directories(include)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(NBody3D main.cpp
        src/octree.cpp
        src/bhtree.cpp)