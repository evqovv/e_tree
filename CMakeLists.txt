cmake_minimum_required(VERSION 3.5.0)
project(e_tree VERSION 0.1.0 LANGUAGES C CXX)

set(CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++")

include_directories($ENV{HOME}/fast_io/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(e_tree ${SRC_FILES})