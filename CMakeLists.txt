cmake_minimum_required(VERSION 3.15)
project(ls_cparser C)

set(CMAKE_C_STANDARD 99)

add_library(ls_cparser INTERFACE)
target_include_directories(ls_cparser INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# For testing, in CLion set `-DLS_CPARSER_TESTS=on` under `Settings > Build, Execution, Deployment > CMake > CMake Options`
if(LS_CPARSER_TESTS)
    add_subdirectory(tests)
endif()