cmake_minimum_required(VERSION 3.14)
project(biquad-ss C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/)

add_executable(biquad_cli cli.c biquad_design.c)

add_executable(biquad_examples examples.c biquad_design.c)
