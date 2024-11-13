@ECHO OFF
ECHO .
FOR /F %%i IN ('DIR /A:D /B %USERPROFILE%\.vscode\extensions\vscode-arduino.vscode-arduino-community-0.*') DO SET AD=%%i
SET C=%USERPROFILE%\.vscode\extensions\%AD%\assets\platform\win32-x64\arduino-cli\arduino-cli.exe

IF NOT EXIST %C% (
    ECHO "ERROR: arduino-cli not found in %C%"
) ELSE (
    %C% %1 %2 %3 %4 %5 %6 %7 %8 %9
)
ECHO .

