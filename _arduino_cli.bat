@ECHO OFF
ECHO .
SET C=%USERPROFILE%\.vscode\extensions\vsciot-vscode.vscode-arduino-0.5.0-win32-x64\assets\platform\win32-x64\arduino-cli\arduino-cli.exe
IF NOT EXIST %C% (
    ECHO "ERROR: arduino-cli not found in %C%"
) ELSE (
    %C% %1 %2 %3 %4 %5 %6 %7 %8 %9
)
ECHO .

