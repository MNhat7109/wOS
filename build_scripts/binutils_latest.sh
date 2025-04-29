#!/bin/sh

FILE_NAME=$(basename $0)
if [ $# -ne 4 ]; then
    echo "Usage: ${FILE_NAME} path/to/dir"
    exit 1
fi

BASE_URL="https://ftp.gnu.org/gnu/binutils/"
LATEST_BINUTILS_FILE=$(curl -s ${BASE_URL} | grep -oP 'binutils-[0-9]+\.[0-9]+(\.[0-9])?\.tar\.xz' | sort -V | tail -n 1)
BINUTILS_DIR=$(echo ${LATEST_BINUTILS_FILE} | sed 's/.tar.xz/\//g')
aria2c "${BASE_URL}${LATEST_BINUTILS_FILE}" -d $(realpath $1) -x 16 --auto-file-renaming=false
tar -xvf $1/${LATEST_BINUTILS_FILE} -C $2
cd $2
./$BINUTILS_DIR/configure --target=$3 --prefix="$4" --with-sysroot --disable-nls --disable-werror
make -C $2 -j8
make -C $2 install

cd ..