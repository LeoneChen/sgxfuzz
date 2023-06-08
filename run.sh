#!/bin/bash
CUR_DIR=$(realpath .)

./kafl/kAFL-Fuzzer/kafl_fuzz.py -kernel /home/leone/Documents/linux/arch/x86_64/boot/bzImage -initrd ${CUR_DIR}/target.cpio.gz -mem 512 -sharedir ${CUR_DIR}/sharedir -seed_dir ${CUR_DIR}/seeds -p 1 -redqueen -redq_do_simple -struct_size_havoc --purge -abort_time 0.1 -w ${CUR_DIR}/workdir_T0 -cpu_affinity_offset 8
# ./kafl/kAFL-Fuzzer/kafl_debug.py --debug -action single -w ${CUR_DIR}/workdir_debug -kernel /home/leone/Documents/linux/arch/x86_64/boot/bzImage -initrd ${CUR_DIR}/target.cpio.gz -mem 512 -sharedir ${CUR_DIR}/sharedir --purge -input ${CUR_DIR}/seeds/seed
