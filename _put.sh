#!/bin/bash

F="main.py"
if [[ -f "$1" ]]; then F=$(basename "$1"); fi

export RSHELL_PORT=/dev/ttyS4
#rshell cp "$F" "/pyboard/$F"
ampy put "$F"
ampy ls
rshell repl