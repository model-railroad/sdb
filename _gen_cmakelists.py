#!/usr/bin/python
#
# Python3 script that generates the CMakeLists.txt file for CLion.

import argparse
import os
import sys

DEST="CMakeLists.txt.tmp"
PARSER = argparse.ArgumentParser()
ARGS = None
VARS = {
    "ARDUINO_DIR": "${WIN_USER_PROFILE}/AppData/Local/Arduino15",
    "XTENSA_DIR": "packages/esp32/tools/xtensa-esp32-elf-gcc/gcc8_4_0-esp-2021r2-patch5",
    "gcc": "xtensa-esp32-elf-gcc.exe",
    "gpp": "xtensa-esp32-elf-g++.exe",
    "cxx_flags": "...",
    "src_headers": "...",
    "src_dirs": "src src/html",
}

CXX_FLAGS_TEMPLATE = """
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} %(flag)s")
"""

MAKE_TEMPLATE = """
cmake_minimum_required(VERSION 3.26)

# Arduino directories
string(REPLACE "\\" "/" WIN_USER_PROFILE $ENV{USERPROFILE})
set(ARDUINO_DIR ${WIN_USER_PROFILE}/%(ARDUINO_DIR)s)
set(XTENSA_DIR ${ARDUINO_DIR}/%(XTENSA_DIR)s)

# Custom toolchain for cross-compiling
# the name of the target operating system
set(CMAKE_SYSTEM_NAME ArduinoEsp32)
# which compilers to use for C and C++
set(CMAKE_C_COMPILER     ${XTENSA_DIR}/bin/%(gcc)s)
set(CMAKE_CXX_COMPILER   ${XTENSA_DIR}/bin/%(gpp)s)
# where is the target environment located
set(CMAKE_FIND_ROOT_PATH ${XTENSA_DIR})
# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# --- SDB Project ---
project(SDB)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force the .ino file to be treated as C++
set_source_files_properties(${PROJECT_SOURCE_DIR}/sdb.ino PROPERTIES LANGUAGE CXX)
set_source_files_properties(${PROJECT_SOURCE_DIR}/sdb.ino PROPERTIES COMPILE_FLAGS "-x c++")

%(cxx_flags)s

add_library(SDB ${PROJECT_SOURCE_DIR}/sdb.ino
%(src_headers)s
)

set_target_properties(SDB PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(SDB PRIVATE
%(src_dirs)s
)

target_include_directories(SDB PRIVATE
%(arduino_dir_includes)s
%(sketch_libs_includes)s
)
# ~~
"""

def init():
    global ARGS
    PARSER.add_argument("-f", "--force",
                        action="store_true",
                        help="force overwriting " + DEST)
    ARGS = PARSER.parse_args()

def check_exists():
    if os.path.isfile(DEST) and not ARGS.force:
        print("%s already exists. Use -f to overwrite it." % DEST)
        sys.exit(1)

def check_prerequisite():
    pass

def generate():
    pass

if __name__ == "__main__":
    init()
    check_exists()
    check_prerequisite()
    generate()
