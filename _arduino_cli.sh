#!/bin/bash
C=( $(cygpath "$USERPROFILE")/.vscode/extensions/vsciot-vscode.vscode-arduino-*-win32-*/assets/platform/win32-*/arduino-cli/arduino-cli.exe )
C="${C[0]}"
if [[ ! -x "$C" ]]; then
    echo "ERROR: arduino-cli not found in $USERPROFILE/.vscode/extensions"
    exit 1
fi
"$C" "$@"

