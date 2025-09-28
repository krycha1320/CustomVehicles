cmake_minimum_required(VERSION 3.10)
project(CustomVehicles)

set(CMAKE_CXX_STANDARD 11)

# ✅ wymuszamy kompilację 32-bit
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

# ✅ nagłówki (tu masz include/amx/amx.h itd.)
include_directories(${CMAKE_SOURCE_DIR}/include)

# ✅ źródła pluginu
add_library(CustomVehicles SHARED
    src/CustomVehicles.cpp
)

# ✅ poprawne nazwy plików wyjściowych
# Linux: CustomVehicles.so
# Windows: CustomVehicles.dll
set_target_properties(CustomVehicles PROPERTIES PREFIX "" SUFFIX ".so")

if (WIN32)
    set_target_properties(CustomVehicles PROPERTIES SUFFIX ".dll")
endif()
