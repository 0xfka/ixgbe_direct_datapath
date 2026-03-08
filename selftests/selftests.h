#ifndef SELFTESTS_H
#define SELFTESTS_H
#include "../src/hw.h"
int ixgbe_run_diagnostic(const struct hw* hw);
int ixgbe_test_mmio(const struct hw* hw);
int ixgbe_test_ds();
#endif