cmake_minimum_required(VERSION 3.5.0)
project(learning_opengl_project VERSION 0.1.0 LANGUAGES C CXX)

add_executable(learning_opengl_project src/main.cpp src/glad.c src/stb_image.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(learning_opengl_project glfw)

