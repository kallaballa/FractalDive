#!/bin/bash

SORTED="`sort -n colors.txt`"
CANDIDATE="`echo "$SORTED" | head -n1`"

function findClosestColors() {
    min=9999999
    diff=0

    echo "$1" | while read hexcolor; do
        diff="`ciff "$2" "$hexcolor" | cut -d"." -f1`"

        if [ $diff -lt $min ]; then
            echo "$hexcolor"
            MIN=$DIFF
        fi
    done
}
while [ -n "$CANDIDATE" ]; do
    HEX="$CANDIDATE"
    RGB="`echo "obase=10; ibase=16; $CANDIDATE" | bc`"
    echo -e "\t0x00${HEX},"
    SORTED="`echo "$SORTED" | fgrep -v "$CANDIDATE"`"
    CANDIDATE="`findClosestColors "$SORTED" "$CANDIDATE" | tail -n 1`"
done | sed '$ s/.$//' | head -n255

