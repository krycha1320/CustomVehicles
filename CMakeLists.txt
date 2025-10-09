cmake_minimum_required(VERSION 3.10)
project(CustomVehicles)

set(CMAKE_CXX_STANDARD 17)

# ============================================
# Wymuszenie 32-bitów i definicji platformy
# ============================================
if (WIN32)
    add_definitions(-DWIN32)
    set(CMAKE_GENERATOR_PLATFORM "Win32" CACHE STRING "" FORCE)
    message(STATUS ">>> Building for Windows (32-bit DLL)")
else()
    add_definitions(-DLINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fPIC -DLINUX")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
    message(STATUS ">>> Building for Linux (32-bit SO)")
endif()

# ============================================
# Ścieżki do nagłówków
# ============================================
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/amx
)

# ============================================
# Źródła
# ============================================
add_library(CustomVehicles SHARED
    src/CustomVehicles.cpp
)

# ============================================
# Nazwy i katalogi wyjściowe
# ============================================
if (WIN32)
    set_target_properties(CustomVehicles PROPERTIES SUFFIX ".dll")
else()
    set_target_properties(CustomVehicles PROPERTIES SUFFIX ".so")
endif()

set_target_properties(CustomVehicles PROPERTIES
    OUTPUT_NAME "CustomVehicles"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
)

# ============================================
# Informacja w konsoli
# ============================================
message(STATUS "============================================")
message(STATUS "Building CustomVehicles plugin")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Output dir: ${CMAKE_SOURCE_DIR}/plugins")
message(STATUS "Target: ${CMAKE_SYSTEM_NAME}")
message(STATUS "============================================")
