cmake_minimum_required(VERSION 3.10)
project(CustomVehicles)

set(CMAKE_CXX_STANDARD 11)

# Wymuszenie 32-bitów
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

# katalogi z nagłówkami
include_directories(include)

# tworzymy bibliotekę współdzieloną
add_library(CustomVehicles SHARED src/CustomVehicles.cpp)

# poprawna nazwa pluginu (CustomVehicles.dll / CustomVehicles.so)
set_target_properties(CustomVehicles PROPERTIES PREFIX "")
