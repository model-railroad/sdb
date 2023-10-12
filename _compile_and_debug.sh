date
pwd
V="$1" #--verbose
./_gen_html_gz.sh src/html/_mod_wifi_ap_index.html
./_gen_html_gz.sh src/html/_mod_wifi_sta_index.html
./_arduino_cli.sh compile $V --upload --profile wifikit32 \
    --build-property "build.extra_flags=-g -O0" \
    --build-property "compiler.cpp.extra_flags=-DESP32" \
    --optimize-for-debug \
&& ./monitor.sh
