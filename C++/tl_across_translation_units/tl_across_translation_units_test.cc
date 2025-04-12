#include <thread>
#include <latch>
#include <vector>

#if defined(OS_LINUX)
#include <fstream>
#include <cstring>
#include <format>
#include <string>
#endif

#include "gtest/gtest.h"
#include "tl_defs.hh"

// Returns the size of data + stack virtual memory for the current process in KiBs. 
static unsigned long vm_self_data_and_stack_size();

#if defined(OS_LINUX)
struct statm {
    unsigned long size;
    unsigned long resident;
    unsigned long share;
    unsigned long text;
    unsigned long lib;
    unsigned long data;
    unsigned long dt;
};

static statm vm_self_statm();
#endif

TEST(tl_across_translation_units, statically_linked_executable) {
    const int THREAD_COUNT = 4;
    std::vector<std::thread> threads;
    std::latch threads_created_latch(THREAD_COUNT);
    std::latch unblock_threads_latch(1);
    
    // get_thread_area() syscall is highly architecture specific,
    // and the standard library doesn't provide a portable wrapper.
    // In addition, we dont want to depend on a specific userspace allocator (like mallinfo()).
    // Furthermore, per thread memory usage is more complex to retrieve,
    // and the total/sum provides similar information. 
    // So, we just check the total system allocated memory for the
    // program.
    // We record the memory usage before the threads creation and 
    // later subtract to improve the accuracy of the test.
    auto static_memory_usage_kib_before = vm_self_data_and_stack_size();

    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back([&threads_created_latch, &unblock_threads_latch] () {
            threads_created_latch.count_down();
            unblock_threads_latch.wait();
        });
    }
    
    threads_created_latch.wait();
    
    long static_memory_usage_kib_after = vm_self_data_and_stack_size();
    ASSERT_GE(static_memory_usage_kib_after - static_memory_usage_kib_before, THREAD_COUNT * TL_SIZE_BYTES / 1024) 
        << "Memory usage including thread local area is less than expected";

    unblock_threads_latch.count_down();
    for (auto& t : threads) {
        t.join();
    }
}

static unsigned long vm_self_data_and_stack_size() {
#if defined(OS_LINUX)
    return vm_self_statm().data;
#else
    throw std::runtime_error("vm_self_data_and_stack_size() is not implemented for this platform");
#endif
}

#if defined(OS_LINUX)
static statm vm_self_statm() {
    auto statm_path = "/proc/self/statm";
    std::ifstream statm_file(statm_path);
    if (!statm_file) {
        throw std::runtime_error(std::format("Failed to open {}", statm_path));
    }

    statm buf;
    statm_file >> buf.size >> buf.resident >> buf.share >> buf.text >> buf.lib >> buf.data >> buf.dt;
    
    // A dummy ' ' for checking bad I/O before isspace() on the second iteration and onwards,
    // and calling fail() only when eof() returns false.
    auto trailing_whitespace = std::string(1, ' ');
    // The first iteration checks the previous data I/O.
    do {
        if (statm_file.bad()) {
            throw std::runtime_error(std::format("I/O error while reading from file {}", statm_path));
        }
        if (statm_file.fail()) {
            throw std::runtime_error(std::format("Non-integer data encountered from file {}", statm_path));
        }
        if (!std::isspace(trailing_whitespace[0])) {
            throw std::runtime_error("Unsupported proc_pid_statm file format");
        }
        statm_file.read(&trailing_whitespace[0], 1);
    } while (!statm_file.eof());
    return buf;
}
#endif
