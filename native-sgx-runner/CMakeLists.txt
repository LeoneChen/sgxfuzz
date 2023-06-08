set(CMAKE_CXX_STANDARD 20)

set(source
		NativeEnclave.cpp
		buffer/GuardedBuffer.cpp
		InputNode.hpp
		)

if (DEFINED ENCLAVE_PATH)
	add_definitions(-DENCLAVE_PATH=${ENCLAVE_PATH})
endif ()
if (DEFINED TCS_PAGE)
	add_definitions(-DTCS_PAGE=${TCS_PAGE})
endif ()
if (DEFINED ENCLAVE_LAYOUT_PATH)
	add_definitions(-DENCLAVE_LAYOUT_PATH=${ENCLAVE_LAYOUT_PATH})
endif ()
if (DEFINED NO_VM_RELOAD)
	add_definitions(-DNO_VM_RELOAD=0${NO_VM_RELOAD})
endif ()

get_filename_component(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
get_filename_component(PACKER_USPACE_SRC_DIR ${ROOT_DIR}/packer/packer/linux_x86_64-userspace/src ABSOLUTE)
get_filename_component(ZYDIS_INSTALL_DIR ${ROOT_DIR}/zydis/install ABSOLUTE)

add_executable(fuzz-generic main-generic.cpp ${source}
    ${PACKER_USPACE_SRC_DIR}/misc/crash_handler.c
    ${PACKER_USPACE_SRC_DIR}/misc/harness_state.c
    ${PACKER_USPACE_SRC_DIR}/misc/struct_synth_report.c
    ${PACKER_USPACE_SRC_DIR}/netfuzz/syscalls.c
)

target_include_directories(fuzz-generic PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${ROOT_DIR}/kAFLUSpaceUtil/include ${PACKER_USPACE_SRC_DIR} ${ZYDIS_INSTALL_DIR}/include)
target_link_libraries(fuzz-generic PRIVATE ${ZYDIS_INSTALL_DIR}/lib/libZydis.a dl nyx_agent)
