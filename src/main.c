#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <xmmintrin.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>

#include "../selftests/selftests.h"
#include "base.h"
#include "datapath_clean.h"
#include "hw.h"
#include "ixgbe.h"
#include "pci.h"
#include "datapath.h"
#include "rx.h"
struct hw ixgbe_adapter __attribute__((aligned(64))) = {0};
static struct ixgbe_stats stats = {0};
static struct pkt_ctx pkt_ctx __attribute__((aligned(64))) = {0};
int main(const int argc, char** argv) {
  int err;
  err = ixgbe_test_ds();
  if (unlikely(err != 0)) {
    return -err;
  }
  if (unlikely(argc < 2)) {
    write(STDERR_FILENO,
          "usage: ./binary <pci_addr>. use lspci for PCI addr.\n", 52);
    return EINVAL;
  }
  if (unlikely(argv[1] == NULL)) {
    return EINVAL;
  }
  ixgbe_adapter.pci_addr = argv[1];
  // Driver should be changed for another PCI direct access modes.
  err = unbind(ixgbe_adapter.pci_addr, "uio_pci_generic");
  if (unlikely(err != 0)) {
    return -err;
  }
  err = alloc_hugepage(&ixgbe_adapter);
  if (unlikely(err != 0)) {
    return -err;
  }
  err = virt2phy((u64)ixgbe_adapter.rx_base,&ixgbe_adapter.rx_base_phy);
  if (unlikely(err != 0)) {
    return -err;
  }
  err = virt2phy((u64)ixgbe_adapter.tx_base,&ixgbe_adapter.tx_base_phy);
  if (unlikely(err != 0)) {
    return -err;
  }
  err = mmap_bar0(&ixgbe_adapter);
  if (unlikely(err != 0)) {
    return -err;
  }
  err = ixgbe_run_diagnostic(&ixgbe_adapter);
  if (unlikely(err != 0)) {
    return -err;
  }
  err = ixgbe_probe(&ixgbe_adapter);
  if (unlikely(err != 0)) {
    return -err;
  }
  union ixgbe_adv_rx_desc *rx_ring = (union ixgbe_adv_rx_desc *)ixgbe_adapter.rx_base;
  union ixgbe_adv_tx_desc *tx_ring = (union ixgbe_adv_tx_desc *)ixgbe_adapter.tx_base;
  ixgbe_write_reg(&ixgbe_adapter, IXGBE_RDT, BUFFER_NUMBER -1);
  ixgbe_read_reg(&ixgbe_adapter, IXGBE_GPRC);
  ixgbe_read_reg(&ixgbe_adapter, IXGBE_RXMPC);
  u32 read_val = ixgbe_read_reg(&ixgbe_adapter, IXGBE_AUTOC);
  IXGBE_SET_BITS(read_val, IXGBE_AUTOC_RESTART);
  ixgbe_write_reg(&ixgbe_adapter, IXGBE_AUTOC, read_val);
  /* This register is used for updating ring buffer location on every x bytes. 
  * 128 is a placeholder, a number will be decided after benchmarks. */
  stats.batch_manage_tail = 128;
  stats.batch_manage_tail_counter = 0;
  stats.batch_tx_counter = 0;
  /* Basic benchmarks show that batching Tx is increasing latency too much.
  * Benchmarks will be added before merging this branch to main.
  */
  stats.batch_tx_transmit = 0;
  u32 i = ixgbe_read_reg(&ixgbe_adapter, IXGBE_RDH);
  pkt_ctx.stats = &stats;
  while(1){
    barrier();
    if(likely(rx_ring[i].wb.status_error & IXGBE_RXD_STAT_DD)){
      rmb();
      stats.batch_manage_tail_counter++;
      stats.total_packets++;
      /* Packet parsing logic is added temporarily to prove pointer arithmatics on structures.
      * Since the driver cannot reply ARP's, static ARP configuration needed.
      */
      u8* pkt = (u8*)ixgbe_adapter.rx_base + (256 * 1024) + ( i * 2048);
      pkt_ctx.pkt = pkt;
      u8 proto = rx_entry(&pkt_ctx, i, rx_ring);
      bool processed = false;
      switch (proto) {
        case PROTO_ICMP: processed = ping_reply(&pkt_ctx,i, rx_ring, tx_ring);
      }
      wmb();
      datapath_clean(&pkt_ctx,processed, i, rx_ring, tx_ring);
      i = IXGBE_BUFFER_ADVANCE(i, 1);
    } 
    }
}