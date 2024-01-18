cmake_minimum_required(VERSION 3.25)
project(NBody3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fopenmp")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx2 -fopenmp -march=native")

find_package(glfw3 3.3 REQUIRED)
find_package(Eigen3 3.3 NO_MODULE)
include_directories(${GLFW_INCLUDE_DIRS})
link_libraries(${GLFW_LIBRARY_DIRS})

include_directories(glad/include include)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(NBody3D main.cpp
        src/octree.cpp
        src/bhtree.cpp
        src/shader.cpp
        src/sphere.cpp
        src/render.cpp
        glad/src/glad.c)

target_link_libraries(${PROJECT_NAME} glfw ${CMAKE_DL_LIBS})