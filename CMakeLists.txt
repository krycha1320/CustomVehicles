cmake_minimum_required(VERSION 3.15)
project(CustomVehicles)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# jeśli masz include/ z SDK, użyj go
include_directories(${CMAKE_SOURCE_DIR}/include)

# Tworzymy bibliotekę współdzieloną (DLL / SO)
add_library(CustomVehicles SHARED src/CustomVehicles.cpp)

# Usuń domyślny 'lib' prefix na Linux, żeby plik był CustomVehicles.so
if (UNIX)
    set_target_properties(CustomVehicles PROPERTIES PREFIX "" SUFFIX ".so")
    set_target_properties(CustomVehicles PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    # Na Windows chcemy plik w build/Release/CustomVehicles.dll (Visual Studio używa Release foldera)
    set_target_properties(CustomVehicles PROPERTIES PREFIX "" RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
endif()

# (Opcjonalne) wymuszenie 32-bitowych flag jeżeli kompilowane lokalnie pod Makefiles
# Nie nadpisujemy ich tu — workflow przekazuje -m32 przy potrzeby na Linuxie.
