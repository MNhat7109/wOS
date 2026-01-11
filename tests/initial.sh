#!/bin/bash

FILE_PATH=$1
QEMU=$2
ITER=$3
FAIL=0

i=1
while [ "$i" -le $ITER ]; do
    printf "[TEST]: Running test $i of $ITER "
    make -s >/dev/null 2>&1 clean && make -s >/dev/null 2>&1 buildimg || exit 1
    make -s >/dev/null 2>&1 test

    if ! echo $(make display_log) | grep -q "BOOT_OK"; then
        printf "[FAILED]\n"
        FAIL=$((FAIL+1))
    else printf "[PASSED]\n"
    fi
    i=$((i+1))
done

if [ "$FAIL" -eq 0 ]; then
    echo "All tests passed"
else echo "$FAIL/$ITER tests failed"
fi