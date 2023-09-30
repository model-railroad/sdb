date
pwd
V="$1" #--verbose
./_gen_html_gz.sh src/_mod_wifi_ap_index.html
./_gen_html_gz.sh src/_mod_wifi_sta_index.html
./_arduino_cli.sh compile $V --upload --profile wifikit32 \
    --build-property "build.extra_flags=-Os" \
    --build-property "compiler.cpp.extra_flags=-DESP32" \
&& ./_arduino_cli.sh monitor -p COM5 -c baudrate=115200
