@ECHO OFF
ECHO %DATE% %TIME%
ECHO %CD%
SET U=--upload
_arduino_cli.bat compile %U% --profile wifikit32 ^
    --build-property "build.extra_flags=-Os" ^
    --build-property "compiler.cpp.extra_flags=-DESP32"
_arduino_cli.sh monitor -p COM5 -c baudrate=115200
