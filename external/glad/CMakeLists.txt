cmake_minimum_required(VERSION 3.10)
project(glad C)

# Create an object library from glad.c
add_library(glad OBJECT src/glad.c)

# Add include directory for glad.h and khrplatform.h
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
