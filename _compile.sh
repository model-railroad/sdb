date
pwd

# Select Profile...
# Override the profile+port by writing its name in a file "sketch.local" (which is not checked in git)
if [[ -f sketch.local ]]; then
    PROF=$(cat sketch.local | awk '{print $1}' | tr -d -c "[a-z0-9]")
    PORT=$(cat sketch.local | awk '{print $2}')
fi
if [[ -z "$PROF" ]]; then
    PROF="wifikit32"
fi
if [[ -z "$PORT" ]]; then
    PORT=$(grep default_port sketch.yaml | awk '{print $2}')
fi

echo "Sketch.yaml profile $PROF on port $PORT"

if [[ -d src/cmake-build-debug ]]; then
    for F in $(find src/cmake-build-debug/ -name "*\.c" -or -name "*\.cpp"); do
        if [[ -f "$F" ]]; then rm -v "$F"; fi
    done
fi


./_gen_html_gz.sh src/html/_mod_wifi_ap_index.html
./_gen_html_gz.sh src/html/_mod_wifi_sta_index.html
./_gen_html_gz.sh src/html/_mod_wifi_style.css

## Arduino-Cli

AC_VERS=$(./_arduino_cli.sh version)
echo "Using $AC_VERS"

## Flags
echo "Updating flags..."

FLAGS="-Os"
if [[ "$1" == "-g" ]]; then
    FLAGS="-g -O0"
    PROF="${PROF}_debug"
    shift
fi

LOG=""
if [[ "$AC_VERS" =~ "Version: 1" ]]; then
    LOG="--log"
fi

# Note that arduino-cli "build-property" _replaces_ values, it doesn't append to them.
# We need to find the default platform values to preserve them, lest we break the compilation.

PROPS=$(./_arduino_cli.sh compile \
    --build-path ./build \
    --profile $PROF --show-properties)

BUILD_FLAGS=$(echo "$PROPS" | grep "^build.extra_flags=" | cut -d = -f 2-)
CPP_FLAGS=$(echo "$PROPS" | grep "^compiler.cpp.extra_flags=" | cut -d = -f 2-)
AE_VERS=$(echo "$PROPS" | grep "^version=" | cut -d = -f 2-)  # arduino-esp32 version

BUILD_FLAGS="-DESP32 -DESP32_PROFILE_$PROF $FLAGS $BUILD_FLAGS"

if [[ "$AE_VERS" =~ "^3\." ]]; then
    # Enable C++20 Module support for arduino-esp32 3.x
    CPP_FLAGS="-fmodules-ts $CPP_FLAGS"
fi

echo "Compiling..."

./_arduino_cli.sh compile \
    $LOG \
    --build-path ./build \
    --profile $PROF \
    -p $PORT \
    --build-property "build.extra_flags=$BUILD_FLAGS" \
    --build-property "compiler.cpp.extra_flags=$CPP_FLAGS" \
    $@
