cmake_minimum_required(VERSION 3.16)
project(SGXFuzz)

add_subdirectory(kAFLUSpaceUtil)
add_subdirectory(native-sgx-runner)

install(TARGETS
    nyx_agent
    nyx_agent_static
    vmcall
    fuzz-generic
)
