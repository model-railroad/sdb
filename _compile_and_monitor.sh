date
pwd
./_arduino_cli.sh compile --upload --profile wifikit32 \
    --build-property "build.extra_flags=-Os" \
    --build-property "compiler.cpp.extra_flags=-DESP32" \
&& ./_arduino_cli.sh monitor -p COM5 -c baudrate=115200
