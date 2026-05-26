cmake_minimum_required(VERSION 3.16)
project(openGl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

add_executable(openGl main.cpp)

target_compile_definitions(openGl
        PRIVATE
        SHADER_DIR="${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(openGl
        PRIVATE
        OpenGL::GL
        GLUT::GLUT
        GLEW::GLEW
)
