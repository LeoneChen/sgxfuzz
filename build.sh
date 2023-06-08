#!/bin/bash
set -e

CUR_DIR=$(realpath -s $(dirname $0))
ENCLAVE="/home/leone/Documents/sgxfuzz/Enclaves/sgx-wallet/enclave.signed.so"

if [[ $1 == 'debug' ]]; then
    CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug"
else
    CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release"
fi

cmake -B build -DENCLAVE_PATH="${ENCLAVE}.mem" -DTCS_PAGE="$(head -1 ${ENCLAVE}.tcs.txt)" -DENCLAVE_LAYOUT_PATH="${ENCLAVE}.layout" -DCMAKE_INSTALL_PREFIX="${CUR_DIR}/install" ${CMAKE_FLAGS}
cmake --build build -j$(nproc)
# cd build
# make VERBOSE=1 -j$(nproc)
# cd ..
cmake --install build
