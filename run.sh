#!/bin/bash

set -ex

SGXFUZZ_ROOT=$(dirname "$(realpath "$0")")
ENCLAVE_PATH="$SGXFUZZ_ROOT/Enclaves/sgx-wallet"
FUZZ_FOLDER=/tmp/sgxfuzz-fuzz-folder
FUZZ_WORKDIR=/tmp/sgxfuzz-workdir

# Build the enclave runner
"$SGXFUZZ_ROOT/initialize-target.sh" SGX_WALLET "$SGXFUZZ_ROOT/Enclaves/sgx-wallet/enclave.signed.so.mem"

cd "$(ls -d SGX_WALLET-T0-*/ | sort -r | head -1)"

./pack.sh

./fuzz.sh
