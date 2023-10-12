date
pwd
FLAGS="-Os"
PROF="wifikit32"
if [[ "$1" == "-g" ]]; then
  FLAGS="-g -O0"
  PROF="${PROF}_debug"
  shift
fi
./_gen_html_gz.sh src/html/_mod_wifi_ap_index.html
./_gen_html_gz.sh src/html/_mod_wifi_sta_index.html
./_arduino_cli.sh compile $@ --build-path ./build --profile $PROF \
    --build-property "build.extra_flags=$FLAGS" \
    --build-property "compiler.cpp.extra_flags=-DESP32"
