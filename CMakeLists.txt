cmake_minimum_required(VERSION 3.6)
project(ex2_new)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(SOURCE_FILES main.cpp uthreads.h uthreads.cpp "example from tirgul.cpp" ex2sanity/exampleFromTirgulCPP.cpp)
add_executable(ex2_new ${SOURCE_FILES})