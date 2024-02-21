DEST_SRC="src/CMakeLists.txt"
DEST_TESTS="tests/CMakeLists.txt"

function error() {
    echo "$*"
    exit 1
}

if [[ -f "$DEST_SRC" && "$1" != "-f" ]]; then
    error "$DEST_SRC already exists. Use -f to force overwrite."
fi

CMD_JSON="build/compile_commands.json"
ARDUINO_DIR="AppData/Local/Arduino15"
XTENSA_DIR="packages/esp32/tools/xtensa-esp32-elf-gcc/gcc8_4_0-esp-2021r2-patch5"
_GCC="xtensa-esp32-elf-gcc.exe"
_GPP="xtensa-esp32-elf-g++.exe"
CXX_FLAGS=""
SRC_HEADERS=""
SRC_DIRS=""
ARDUINO_DIR_INCLUDES=""

UP=$(cygpath "$USERPROFILE")

if [[ ! -f "$CMD_JSON" ]] || ! grep --quiet  "g++" build/compile_commands.json; then
    error "Run ./compile.sh to generate $CMD_JSON."
fi

ARDUINO_DIR=( $UP/AppData/Local/Arduino* )
ARDUINO_DIR="${ARDUINO_DIR[0]/$UP\//}"

_GPP=$(cygpath $(grep "g++" build/compile_commands.json | sort -u | head -n 1 | tr -d ' \",') )
XTENSA_DIR=$(dirname "$_GPP")
if [[ $(basename "$XTENSA_DIR") == "bin" ]]; then XTENSA_DIR=$(dirname "$XTENSA_DIR"); fi
XTENSA_DIR="${XTENSA_DIR/$UP\/$ARDUINO_DIR\//}"
_GPP=$(basename "$_GPP")
_GCC="${_GPP//\+\+/cc}"

if [[ ! "$_GCC" =~ .exe$ ]]; then _GCC="${_GCC}.exe"; fi
if [[ ! "$_GPP" =~ .exe$ ]]; then _GPP="${_GPP}.exe"; fi

for F in $(grep '^   "-[a-z_DW]' $CMD_JSON | sort -u | tr -d '\\\", '); do
    CXX_FLAGS="$CXX_FLAGS
set(CMAKE_CXX_FLAGS \"\${CMAKE_CXX_FLAGS} $F\")"
done

echo "Parsing src/, tests/, and build/compile_commands.json..."

SRC_HEADERS=$(cd src ; find . -name "*.h")

SRC_DIRS=$(cd src ; find . -type d | grep -v "\\.$")

TESTS_HEADERS=$(cd tests ; find . -name "*.h" ; find ../src -name "*.h")

TESTS_DIRS=$(cd tests ; find ../src -type d)


for I in $(grep '"-I' build/compile_commands.json | sort -u | sed 's/-I//g' | tr -d '\", '); do
    I=$(cygpath "$I")
    I="${I/$UP\/$ARDUINO_DIR\//}"
    ARDUINO_DIR_INCLUDES="$ARDUINO_DIR_INCLUDES
\${ARDUINO_DIR}/$I"
done

echo "Generating $DEST_SRC"
cat > $DEST_SRC <<EOF
cmake_minimum_required(VERSION 3.26)

# --- SDB Project ---
project(SDB)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Arduino directories
string(REPLACE "\\\\" "/" WIN_USER_PROFILE \$ENV{USERPROFILE})
set(ARDUINO_DIR \${WIN_USER_PROFILE}/$ARDUINO_DIR)
set(XTENSA_DIR \${ARDUINO_DIR}/$XTENSA_DIR)

# Custom toolchain for cross-compiling
# the name of the target operating system
set(CMAKE_SYSTEM_NAME ArduinoEsp32)
# which compilers to use for C and C++
set(CMAKE_C_COMPILER     \${XTENSA_DIR}/bin/$_GCC)
set(CMAKE_CXX_COMPILER   \${XTENSA_DIR}/bin/$_GPP)
# where is the target environment located
set(CMAKE_FIND_ROOT_PATH \${XTENSA_DIR})
# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# --- SRC target ---

# Force the .ino file to be treated as C++
set_source_files_properties(\${PROJECT_SOURCE_DIR}/../sdb.ino PROPERTIES LANGUAGE CXX)
set_source_files_properties(\${PROJECT_SOURCE_DIR}/../sdb.ino PROPERTIES COMPILE_FLAGS "-x c++")

$CXX_FLAGS

add_library(src \${PROJECT_SOURCE_DIR}/../sdb.ino
$SRC_HEADERS
)

set_target_properties(src PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(src PRIVATE
$SRC_DIRS
)

target_include_directories(src PRIVATE
$ARDUINO_DIR_INCLUDES
)
# ~~
EOF

echo "Generating $DEST_TESTS"
cat > $DEST_TESTS <<EOF
cmake_minimum_required(VERSION 3.26)

# --- SDB TESTS Project ---
project(SDB)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_SYSTEM_NAME MSYS)

# --- TESTS target ---

add_executable(tests main_test.cpp
$TEST_HEADERS
)

target_include_directories(tests PRIVATE
$TESTS_DIRS
)

target_include_directories(tests SYSTEM PUBLIC
    mocks
)

# ~~
EOF
