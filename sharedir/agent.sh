ifconfig eth0 10.0.2.15 2>&1 | vmcall hcat
ip addr 2>&1 | vmcall hcat
cd /fuzz
export LD_LIBRARY_PATH=/fuzz/
export LD_BIND_NOW=1
export ASAN_OPTIONS=detect_leaks=0:allocator_may_return_null=1:log_path=/tmp/data.log:abort_on_error=true
export MALLOC_CHECK_=2
# ./gdbserver 127.0.0.1:1234 ./fuzz-generic 2>&1 | vmcall hcat
./fuzz-generic 2>&1 | vmcall hcat
vmcall habort