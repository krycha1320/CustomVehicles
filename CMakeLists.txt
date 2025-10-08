cmake_minimum_required(VERSION 3.10)
project(CustomVehicles)

# ==============================
# ⚙️ Ustawienia kompilatora
# ==============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Wymuszenie 32-bitowej kompilacji
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

# ==============================
# 📁 Ścieżki do plików nagłówkowych
# ==============================
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# ==============================
# 🧱 Pliki źródłowe pluginu
# ==============================
add_library(CustomVehicles SHARED
    src/CustomVehicles.cpp
)

# ==============================
# 💾 Ustawienia nazw i rozszerzeń
# ==============================
if (WIN32)
    set_target_properties(CustomVehicles PROPERTIES
        OUTPUT_NAME "CustomVehicles"
        SUFFIX ".dll"
    )
elseif(UNIX)
    set_target_properties(CustomVehicles PROPERTIES
        OUTPUT_NAME "CustomVehicles"
        SUFFIX ".so"
    )
endif()

# ==============================
# 📂 Folder wyjściowy
# ==============================
set_target_properties(CustomVehicles PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/plugins"
)

# ==============================
# 🧹 Komunikaty pomocnicze
# ==============================
message(STATUS "============================================")
message(STATUS "Building CustomVehicles plugin")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Architecture: 32-bit")
message(STATUS "Output directory: ${PROJECT_SOURCE_DIR}/plugins")
message(STATUS "Output file: CustomVehicles.dll (Windows) or CustomVehicles.so (Linux)")
message(STATUS "============================================")
