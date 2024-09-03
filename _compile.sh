date
pwd
FLAGS="-Os"

# Select Profile from sketch.yaml
# PROF="wifikit32"
PROF="esp32cam"
if [[ "$1" == "-g" ]]; then
  FLAGS="-g -O0"
  PROF="${PROF}_debug"
  shift
fi

if [[ -d src/cmake-build-debug ]]; then
  for F in $(find src/cmake-build-debug/ -name "*\.c" -or -name "*\.cpp"); do
    if [[ -f "$F" ]]; then rm -v "$F"; fi
  done
fi

./_gen_html_gz.sh src/html/_mod_wifi_ap_index.html
./_gen_html_gz.sh src/html/_mod_wifi_sta_index.html
./_gen_html_gz.sh src/html/_mod_wifi_style.css
./_arduino_cli.sh compile $@ --build-path ./build --profile $PROF \
    --build-property "build.extra_flags=$FLAGS" \
    --build-property "compiler.cpp.extra_flags=-DESP32" \
    --build-property "compiler.cpp.extra_flags=-DESP32_PROFILE_$PROF"
