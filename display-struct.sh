#!/bin/bash
SGXFUZZ_ROOT=${PWD}
cd $1

for i in {0..20}; do 
	echo "ECALL " $i
	python3 ${SGXFUZZ_ROOT}/display-structs.py sgx_workdir/ $i | uniq
done

cd ..
