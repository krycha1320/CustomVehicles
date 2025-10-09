cmake_minimum_required(VERSION 3.10)
project(CustomVehicles)

# ==============================
#  Kompilacja 32-bit pluginu
# ==============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fPIC")

# ==============================
#  Ścieżki do nagłówków
# ==============================
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/amx
)

# ==============================
#  Budowanie pluginu
# ==============================
add_library(CustomVehicles SHARED
    src/CustomVehicles.cpp
)

# ==============================
#  Nazwa wyjściowa (dla Linux)
# ==============================
set_target_properties(CustomVehicles PROPERTIES
    OUTPUT_NAME "CustomVehicles"
    PREFIX ""
    SUFFIX ".so"
)

# ==============================
#  Dodatkowy build Windows (opcjonalny)
# ==============================
if (WIN32)
    set_target_properties(CustomVehicles PROPERTIES SUFFIX ".dll")
endif()

message(STATUS "============================================")
message(STATUS " Building CustomVehicles plugin")
message(STATUS " Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS " Architecture:
