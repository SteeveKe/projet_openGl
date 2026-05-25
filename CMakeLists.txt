cmake_minimum_required(VERSION 3.16)
project(openGl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(openGl main.cpp)

target_link_libraries(openGl
        PRIVATE
        OpenGL::GL
        GLUT::GLUT
)
