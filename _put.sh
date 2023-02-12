#!/bin/bash
export RSHELL_PORT=/dev/ttyS4

FS="src/main.py"

if [[ -f "$1" ]]; then
    for f in "$@"; do
        echo "Put $f"
        ampy put "$f"
    done
else
    f="$FS"
    echo "Put $f"
    ampy put "$f"
fi


ampy ls
rshell repl
