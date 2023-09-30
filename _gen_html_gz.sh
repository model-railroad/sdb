#!/bin/bash
set -e

HTML="$1"
NAME=$(basename "$HTML")
NAME="${NAME%%.html}"
NAME="${NAME##_}"
OUT="${HTML}.gz.h"

if [[ ! -f "$HTML" ]]; then
    echo "Error: Missing $HTML file."
    exit 1
fi

GZ=$(mktemp --tmpdir "html.XXXXXX.gz")
BYTESGZ="$GZ.h"
gzip --best --to-stdout "$HTML" > "$GZ"
cat "$GZ" | xxd -i > "$BYTESGZ"

SIZE=$(stat -c%s "$HTML")
SIZEGZ=$(stat -c%s "$GZ")
sed -e "s/@NAME@/$NAME/g; s/@SIZE@/$SIZE/g; s/@SIZEGZ@/$SIZEGZ/g" \
    -e "/@BYTESGZ@/e cat $BYTESGZ" \
    -e "/@BYTESGZ@/d" \
    src/_mod_wifi_index_template.h > $OUT

rm "$GZ"
rm "$BYTESGZ"

COMP=$(python -c "print('{:.2f}'.format(100*$SIZEGZ./$SIZE))")
echo "Converted $HTML ($SIZE bytes) to $OUT ($SIZEGZ bytes, $COMP% of original)"
