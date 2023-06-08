#!/bin/bash
CUR_DIR=$(realpath .)

${CUR_DIR}/kAFLUSpaceUtil/initrd/gen_initrd.sh ${CUR_DIR}/target.cpio.gz ${CUR_DIR}/install/bin/vmcall ${CUR_DIR}/install/bin/fuzz-generic ${CUR_DIR}/install/lib/libnyx_agent.so /usr/bin/addr2line /usr/bin/gdbserver