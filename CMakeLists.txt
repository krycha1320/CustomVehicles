cmake_minimum_required(VERSION 3.10)
project(CustomVehicles)

set(CMAKE_CXX_STANDARD 11)

# wymuś 32-bit
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

# powiedz plugincommon.h, że budujemy na Linuxie
add_definitions(-DLINUX)

# katalog z nagłówkami
include_directories(${CMAKE_SOURCE_DIR}/include)

# źródła
add_library(CustomVehicles SHARED
    src/CustomVehicles.cpp
)

# rozszerzenia plików wyjściowych
set_target_properties(CustomVehicles PROPERTIES PREFIX "" SUFFIX ".so")

if (WIN32)
    add_definitions(-DWIN32)
    set_target_properties(CustomVehicles PROPERTIES SUFFIX ".dll")
endif()
