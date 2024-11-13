date
pwd
FLAGS="-Os"

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

if [[ "$1" == "-g" ]]; then
  FLAGS="-g -O0"
  PROF="${PROF}_debug"
  shift
fi

echo "Sketch.yaml profile $PROF on port $PORT"

if [[ -d src/cmake-build-debug ]]; then
  for F in $(find src/cmake-build-debug/ -name "*\.c" -or -name "*\.cpp"); do
    if [[ -f "$F" ]]; then rm -v "$F"; fi
  done
fi

LOG=""
if [[ $(./_arduino_cli.sh version) =~ "Version: 1" ]]; then
  LOG="--log"
fi

./_gen_html_gz.sh src/html/_mod_wifi_ap_index.html
./_gen_html_gz.sh src/html/_mod_wifi_sta_index.html
./_gen_html_gz.sh src/html/_mod_wifi_style.css
./_arduino_cli.sh compile \
    $LOG \
    --build-path ./build \
    --profile $PROF \
    -p $PORT \
    --build-property "build.extra_flags=$FLAGS" \
    --build-property "build.extra_flags=-DESP32" \
    --build-property "build.extra_flags=-DESP32_PROFILE_$PROF" \
    $@
