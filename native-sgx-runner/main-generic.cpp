#include "NativeEnclave.h"
#include <stdexcept>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "InputNode.hpp"

extern "C" {
#include "nyx.h"
#include "nyx_agent.h"
#include "misc/crash_handler.h"
#include "misc/harness_state.h"
}

#if !defined(ENCLAVE_PATH) || !defined(TCS_PAGE)
#error "Please define ENCLAVE_PATH and TCS_PAGE"
#endif

#if NO_VM_RELOAD > 0
#warning "No VM Reload"
#endif

LINK_ENCLAVE(enclave, ENCLAVE_PATH)
LINK_DATA(enclave_layout, ENCLAVE_LAYOUT_PATH, ".encl_layout", "0", "a")

size_t ClMaxPayloadSize = 10000000;

/* struct FastReloadHelper {
    FastReloadHelper() {
        nyx_init();
    }
    ~FastReloadHelper() {
        // Exit the program here and reload the nyx_init state
        kAFL_hypercall(HYPERCALL_KAFL_RELEASE, 0);
    }
}; */

[[nodiscard]] int read_payload(char* buf, int buf_len, unsigned char& ecall_id, InputNode& in, size_t* input_size = nullptr) {
    /*
    constexpr size_t buf_cap = 0x1000;
    char buf[buf_cap];
    ssize_t buf_len = read(fd, buf, sizeof(buf));
     */

    if (buf_len <= 0)
        return -3;

    if (input_size)
        *input_size = buf_len;

    ecall_id = buf[0];
    --buf_len;
    char* struct_data = buf + 1;

    size_t payload_offset = in.deserialize(struct_data) - struct_data;
    if (struct_data[payload_offset++] != ' ')
        return -4;

    if (in.getDataSize() > buf_len - payload_offset) {
        printf("Needs %zu bytes input\n", in.getDataSize());
        return -5;
    }

    in.fillWithData(struct_data + payload_offset);

#if NO_VM_RELOAD > 0
    puts(in.serialize_extended().c_str());
    in.show();
#endif

    return 0;
}

int agent_init(int verbose) {
  // Handshake with front end
  kAFL_hypercall(HYPERCALL_KAFL_ACQUIRE, 0);
  kAFL_hypercall(HYPERCALL_KAFL_RELEASE, 0);

  get_nyx_cpu_type();

  // Get host config
  host_config_t host_config;
  kAFL_hypercall(HYPERCALL_KAFL_GET_HOST_CONFIG, (uintptr_t)&host_config);

  if (verbose) {
    fprintf(stderr, "GET_HOST_CONFIG\n");
    fprintf(stderr, "\thost magic:  0x%x, version: 0x%x\n",
            host_config.host_magic, host_config.host_version);
    fprintf(stderr, "\tbitmap size: 0x%x, ijon:    0x%x\n",
            host_config.bitmap_size, host_config.ijon_bitmap_size);
    fprintf(stderr, "\tpayload size: %u KB\n",
            host_config.payload_buffer_size / 1024);
    fprintf(stderr, "\tworker id: %d\n", host_config.worker_id);
  }

  if (host_config.host_magic != NYX_HOST_MAGIC) {
    hprintf("HOST_MAGIC mismatch: %08x != %08x\n", host_config.host_magic,
            NYX_HOST_MAGIC);
    habort((char *)"HOST_MAGIC mismatch!");
    return -1;
  }

  if (host_config.host_version != NYX_HOST_VERSION) {
    hprintf("HOST_VERSION mismatch: %08x != %08x\n", host_config.host_version,
            NYX_HOST_VERSION);
    habort((char *)"HOST_VERSION mismatch!");
    return -1;
  }

  if (host_config.payload_buffer_size > ClMaxPayloadSize) {
    hprintf("Fuzzer payload size too large: %lu > %lu\n",
            host_config.payload_buffer_size, ClMaxPayloadSize);
    habort((char *)"Host payload size too large!");
    return -1;
  }

  agent_config_t agent_config = {0};
  agent_config.agent_magic = NYX_AGENT_MAGIC;
  agent_config.agent_version = NYX_AGENT_VERSION;
  // agent_config.agent_timeout_detection = 0; // timeout by host
  // agent_config.agent_tracing = 0; // trace by host
  // agent_config.agent_ijon_tracing = 0; // no IJON
  agent_config.agent_non_reload_mode = 0; // no persistent mode
  // agent_config.trace_buffer_vaddr = 0xdeadbeef;
  // agent_config.ijon_trace_buffer_vaddr = 0xdeadbeef;
  agent_config.coverage_bitmap_size = host_config.bitmap_size;
  // agent_config.input_buffer_size;
  // agent_config.dump_payloads; // set by hypervisor (??)

  kAFL_hypercall(HYPERCALL_KAFL_SET_AGENT_CONFIG, (uintptr_t)&agent_config);

  return 0;
}

extern "C" void report_struct_synth_addrs(const char* addrs, ssize_t len) {
    set_struct_synth_addrs(addrs, len);
}
 
int run_ecall(char* init_call_file) {
#if NO_VM_RELOAD == 0
    agent_init(1);
    hprintf("Enclave: Base=%016p Size=0x%016lx\n", enclave_start,
          &enclave_end - enclave_start);
#endif
    NativeEnclave enc(enclave_start, &enclave_end - enclave_start);
    enc.apply_layout((layout_entry*) enclave_layout_start);
    if (!enc.init_entry(TCS_PAGE))
        return -1;

    /* 
    if (init_call_file) {
        int init_fd = open(init_call_file, O_RDONLY);
        if (init_fd < 0) {
            printf("Failed to open init input file %s\n", init_call_file);
            return -82;
        }

        unsigned char init_ecall_id = -10;
        InputNode init_in(&enc, 'C');
        if (int err = read_payload(init_fd, init_ecall_id, init_in)) {
            printf("Failed to read init input file %s\n", init_call_file);
            return err - 80;
        }

        const uint64_t init_ocall = enc.ecall_entry_intel_sdk(init_ecall_id, init_in.generate());
        // TODO: Do something with init_ocall?
    }
     */

#if 0
    {
        /* configure range_a */
        kAFL_hypercall(HYPERCALL_KAFL_RANGE_SUBMIT, (const uintptr_t) (const uint64_t[])
            { (uint64_t) enclave_start, (uint64_t) & enclave_end, 0 }
        );
        /* disable range_b */
        kAFL_hypercall(HYPERCALL_KAFL_RANGE_SUBMIT, (const uintptr_t) (const uint64_t[])
            { 0xFFFFFFFFFFFFF001L, 0XFFFFFFFFFFFFF002L, 1 }
        );

        kafl_dump_file_t file_obj = { (uint64_t) "enclave_addr.range", 0, 0, 0 };
        kAFL_hypercall(HYPERCALL_KAFL_DUMP_FILE, (uint64_t)(&file_obj));

        char range_string[100]{};
        snprintf(range_string, 100, "%lx-%lx\n", (uint64_t) enclave_start, (uint64_t) & enclave_end);

        file_obj.append = 1;
        file_obj.bytes = 100;
        file_obj.data_ptr = (uint64_t) range_string;
        kAFL_hypercall(HYPERCALL_KAFL_DUMP_FILE, (uint64_t)(&file_obj));

        /* datei anlegen */
        file_obj = { (uint64_t) "enclave_dump.bin", 0, 0, 0 };
        kAFL_hypercall(HYPERCALL_KAFL_DUMP_FILE, (uint64_t)(&file_obj));

        /* daten schreiben */
        file_obj.append = 1;
        file_obj.bytes = 100;
        file_obj.data_ptr = (uint64_t) & enclave_start;
        kAFL_hypercall(HYPERCALL_KAFL_DUMP_FILE, (uint64_t)(&file_obj));
    }
#endif

    {
#if 0
        // The program is restored to this point/state when exiting
        FastReloadHelper helper;
#endif
        get_harness_state();
        init_crash_handling();

        hrange_submit(0, (uintptr_t)enclave_start, (uintptr_t)&enclave_end);
        kAFL_payload *pbuf =
            (kAFL_payload *)malloc_resident_pages(ClMaxPayloadSize / PAGE_SIZE);
        assert(pbuf);

        kAFL_hypercall(HYPERCALL_KAFL_GET_PAYLOAD, (uint64_t)pbuf);
        kAFL_hypercall(HYPERCALL_KAFL_USER_FAST_ACQUIRE, 0);
        hprintf("Data %p %d\n", pbuf->data, pbuf->size);

        InputNode in(&enc, 'C');
        unsigned char ecall_id = -10;
        size_t input_size = 0;
        if (int err = read_payload((char*)(pbuf->data), pbuf->size, ecall_id, in, &input_size)) {
#if NO_VM_RELOAD == 0
            if (err == -5) {
                auto ext_struct = in.serialize_extended();
                char* starved_report = new char[0x200];
                sprintf(starved_report, "STARVED|%zu|%zu|%s", in.getDataSize(), input_size, ext_struct.c_str());
                kAFL_hypercall(HYPERCALL_KAFL_PANIC_EXTENDED, (uint64_t) starved_report);
            }
#endif
            return err - 20;
        }

        auto ext_struct = in.serialize_extended();
        report_struct_synth_addrs(ext_struct.c_str(), ext_struct.length());

        hprintf("Try ECALL %d\n", ecall_id);
        uint64_t ocall = enc.ecall_entry_intel_sdk(ecall_id, in.generate());
        kAFL_hypercall(HYPERCALL_KAFL_RELEASE, 0);
        return (int) ocall + 1;
    }
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        printf("Reading %s as init call input\n", argv[1]);
        return run_ecall(argv[1]);
    } else {
        return run_ecall(nullptr);
    }
}
