#!/bin/bash

set -ex

SGXFUZZ_ROOT=$(dirname "$(realpath "$0")")
ENCLAVE_PATH="$SGXFUZZ_ROOT/Enclaves/sgx-wallet"
FUZZ_FOLDER=/tmp/sgxfuzz-fuzz-folder
FUZZ_WORKDIR=/tmp/sgxfuzz-workdir

if [[ ! -d "$SGXFUZZ_ROOT/native-sgx-runner" ]]; then
	echo "Invalid execution directory"
	exit 1
fi

if [[ -r "$SGXFUZZ_ROOT/kvm-nyx-release/kvm-intel.ko" ]]; then
	sudo rmmod kvm_intel || true
	sudo rmmod kvm || true
	sudo insmod "$SGXFUZZ_ROOT/kvm-nyx-release/kvm.ko" || true
	sudo insmod "$SGXFUZZ_ROOT/kvm-nyx-release/kvm-intel.ko" || true
	sudo chmod a+rw /dev/kvm
fi

# Build the enclave runner
"$SGXFUZZ_ROOT/initialize-target.sh" sgx-wallet "$SGXFUZZ_ROOT/Enclaves/sgx-wallet/enclave.signed.so.mem"

cd "$(ls -d sgx-wallet-T0-*/ | sort -r | head -1)"

./pack.sh

./fuzz.sh
