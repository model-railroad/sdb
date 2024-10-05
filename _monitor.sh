ADDR2LINE=$(grep "g++" build/compile_commands.json | sort -u | head -n 1 | tr -d \", | sed 's/g++/addr2line/')
if [[ "$1" =~ COM* ]]; then
  PORT="$1"
else
  PORT=$(cat sketch.local | awk '{print $2}')
  if [[ -z "$PORT" ]]; then
    PORT=$(grep default_port sketch.yaml | awk '{print $2}')
  fi
fi
echo "Monitor on $PORT"
./_arduino_cli.sh monitor -p $PORT -c baudrate=115200 2>&1 | while read -r LINE ; do
  echo "$LINE"
  if [[ "$LINE" =~ ^Backtrace: ]]; then
    echo
    $ADDR2LINE -pfiaC -e build/sdb.ino.elf ${LINE:10}
    echo
    echo "Paused 5 seconds"
    sleep 5s
    echo
  fi
done
