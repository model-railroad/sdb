cmake_minimum_required(VERSION 3.26)

# --- SDB Project ---
project(SDB)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

message("@@ CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")

add_subdirectory(tests)
add_subdirectory(src)

# ~~
