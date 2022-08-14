#ifdef RUN_TESTS

#include "Tests/kernel_tests.h"

#include "Kernel/printk.h"

#define TEST_NAME "printk"

void printk_tests() {
    test_start_message(TEST_NAME);

    debugk("%i\n", 0);
}

#endif
