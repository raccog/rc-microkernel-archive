#ifdef RUN_TESTS

#include "Tests/kernel_tests.h"
#include "Kernel/printk.h"

#include "all_tests.h"

#define TEST_SEPARATOR                                                         \
    "=============================================================="           \
    "========\n"

void test_start_message(const char *test_name) {
    debugk(TEST_SEPARATOR);
    debugk("    %s\n", test_name);
    debugk(TEST_SEPARATOR);
}

void run_kernel_tests() {
    debugk("Running all kernel tests and driving output to serial...\n");

    /* all kernel tests */
    printk_tests();
}

#endif
