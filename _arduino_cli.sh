#!/bin/bash

# Search for Arduino CLI in the exec PATH
C=$(which arduino-cli.exe)

if [[ ! -x "$C" ]]; then
	C=$(which arduino-cli)
fi

if [[ ! -x "$C" ]]; then
	# VS Code Arduino Extension by MS:     vsciot-vscode.vscode-arduino
	# VS Code Arduino Community Extension: vscode-arduino.vscode-arduino-community
	C=( $(cygpath "$USERPROFILE")/.vscode/extensions/*.vscode-arduino-*-win32-*/assets/platform/win32-*/arduino-cli/arduino-cli* )
	C="${C[0]}"
fi
if [[ ! -x "$C" ]]; then
    echo "ERROR: arduino-cli not found in $USERPROFILE/.vscode/extensions"
    exit 1
fi
"$C" version
"$C" "$@"

