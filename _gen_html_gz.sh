#!/bin/bash
set -e

HTML="$1"
NAME=$(basename "$HTML")
EXT="${NAME##*.}"
NAME="${NAME%%.$EXT}"
NAME="${NAME##_}"
OUT="${HTML}.gz.h"

if [[ ! -f "$HTML" ]]; then
    echo "Error: Missing $HTML file."
    exit 1
fi

GZ=$(mktemp --tmpdir "html.XXXXXX.gz")
BYTESGZ="$GZ.h"
# gzip normally inserts name+timestamp, resulting in files that change even if same content.
# use --no-name to prevent that from being included.
gzip --no-name --best --to-stdout "$HTML" > "$GZ"
cat "$GZ" | xxd -i > "$BYTESGZ"

SIZE=$(stat -c%s "$HTML")
SIZEGZ=$(stat -c%s "$GZ")
sed -e "s/@NAME@/$NAME/g; s/@SIZE@/$SIZE/g; s/@SIZEGZ@/$SIZEGZ/g" \
    -e "/@BYTESGZ@/e cat $BYTESGZ" \
    -e "/@BYTESGZ@/d" \
    src/html/_mod_wifi_index.h.template.txt > $OUT

rm "$GZ"
rm "$BYTESGZ"

COMP=$(python -c "print('{:.2f}'.format(100*$SIZEGZ./$SIZE))")
echo "Converted $HTML ($SIZE bytes) to $OUT ($SIZEGZ bytes, $COMP% of original)"
