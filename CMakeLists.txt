cmake_minimum_required(VERSION 3.16)
project(rdcsv C)

set(CMAKE_C_STANDARD 90)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)

file(GLOB SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

