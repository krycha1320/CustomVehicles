cmake_minimum_required(VERSION 3.10)
project(CustomVehicles)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fPIC")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/amx
)

add_library(CustomVehicles SHARED
    src/CustomVehicles.cpp
)

set_target_properties(CustomVehicles PROPERTIES
    OUTPUT_NAME "CustomVehicles"
    PREFIX ""
    SUFFIX ".so"
)
