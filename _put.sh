#!/bin/bash

FS="src/main.py"
if [[ -f "$1" ]]; then FS="$1"; fi

export RSHELL_PORT=/dev/ttyS4
ampy put "$FS"
ampy ls
rshell repl
