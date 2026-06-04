cmake_minimum_required(VERSION 3.16)
project(openGl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

add_executable(openGl
        main.cpp
        src/MathUtils.cpp
        src/ObjLoader.cpp
        src/Shader.cpp
        src/Texture.cpp
        src/FrameBuffer.cpp
        src/FrameBuffer.h
        src/GrassGenerator.cpp
        src/TerrainGenerator.cpp
)

target_include_directories(openGl
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib
)

target_compile_definitions(openGl
        PRIVATE
        SHADER_DIR="${CMAKE_SOURCE_DIR}/src/shader"
        MODEL_DIR="${CMAKE_SOURCE_DIR}/models"
)

target_link_libraries(openGl
        PRIVATE
        OpenGL::GL
        GLUT::GLUT
        GLEW::GLEW
)
