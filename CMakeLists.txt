cmake_minimum_required(VERSION 3.15)
project(ls_cparser CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CXX_EXTENSIONS OFF)

add_library(ls_cparser INTERFACE)
target_include_directories(ls_cparser INTERFACE sources)