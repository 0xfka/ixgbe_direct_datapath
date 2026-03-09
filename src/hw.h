#ifndef HW_H
#define HW_H

#include "base.h"
struct hw {
  const char* pci_addr;
  u8* hw_addr;
  void* rx_base;
  u64 rx_base_phy;
  u32 rx_head;
  u32 rx_tail;
  void* tx_base;
  volatile u32* rdt_reg_addr;
  volatile u32* rdh_reg_addr;
};
extern struct hw ixgbe_adapter;

int alloc_hugepage(struct hw* hw);
int virt2phy(u64 enter_value, u64* return_value);
int mmap_bar0(struct hw* hw);
#endif