#!/bin/sh
FILE_NAME=$(basename $0)
if [ $# -ne 4 ]; then
    echo "Usage: ${FILE_NAME} path/to/dir"
    exit 1
fi

BASE_URL="https://ftp.gnu.org/gnu/gcc/"
LATEST_DIR=$(curl -s ${BASE_URL} | grep -oP 'gcc-[0-9]+\.[0-9]+(\.[0-9])?/' | sort -V | tail -n 1)
LATEST_GCC_FILE=$(echo ${LATEST_DIR} | sed 's/\//.tar.xz/g')

aria2c "${BASE_URL}${LATEST_DIR}${LATEST_GCC_FILE}" -d $(realpath $1) -x 16 --auto-file-renaming=false
tar -xvf $1/${LATEST_GCC_FILE} -C $2
cd $2
./$LATEST_DIR/configure --target=$3 --prefix="$4" --disable-nls --disable-werror \
    --enable-languages=c,c++ --without-headers
make -j8 all-gcc all-target-libgcc
make install-gcc install-target-libgcc

cd ..