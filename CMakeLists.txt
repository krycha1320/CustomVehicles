cmake_minimum_required(VERSION 3.10)
project(CustomVehicles LANGUAGES C CXX)

# ===============================
# 🔧 WYMUSZENIE 32-bit KOMPILECJI
# ===============================
if (WIN32)
    set(CMAKE_GENERATOR_PLATFORM "Win32" CACHE STRING "" FORCE)
    set(BUILD_PLATFORM "Windows (32-bit DLL)")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
else()
    set(BUILD_PLATFORM "Linux (32-bit SO)")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()

# ===============================
# 📦 KATALOGI
# ===============================
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_SOURCE_DIR}/include")
set(OUT_DIR "${CMAKE_SOURCE_DIR}/plugins")

file(MAKE_DIRECTORY ${OUT_DIR})

# ===============================
# 📁 PLIKI ŹRÓDŁOWE
# ===============================
set(SOURCES
    ${SRC_DIR}/CustomVehicles.cpp
)

# ===============================
# ⚙️ USTAWIENIA KOMPILATORA
# ===============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(LINUX)

include_directories(
    ${INC_DIR}
    ${INC_DIR}/amx
)

# ===============================
# 🏗️ TWORZENIE BIBLIOTEKI
# ===============================
add_library(CustomVehicles SHARED ${SOURCES})

set_target_properties(CustomVehicles PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR}
)

# ===============================
# 💾 NAZWY WYJŚCIOWE
# ===============================
if (WIN32)
    set_target_properties(CustomVehicles PROPERTIES OUTPUT_NAME "CustomVehicles")
    set_target_properties(CustomVehicles PROPERTIES SUFFIX ".dll")
else()
    set_target_properties(CustomVehicles PROPERTIES OUTPUT_NAME "CustomVehicles")
    set_target_properties(CustomVehicles PROPERTIES SUFFIX ".so")
endif()

# ===============================
# 🧩 DODATKOWE INFO W LOGACH
# ===============================
message(STATUS "")
message(STATUS ">>> Building for ${BUILD_PLATFORM}")
message(STATUS "============================================")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Output dir: ${OUT_DIR}")
message(STATUS "Target: ${BUILD_PLATFORM}")
message(STATUS "============================================")

# ===============================
# ✅ SUKCES
# ===============================
message(STATUS "✔ CMake configuration complete. Ready to build.")
