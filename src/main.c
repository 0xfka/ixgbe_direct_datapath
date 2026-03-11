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
#include "hw.h"
#include "ixgbe.h"
#include "pci.h"
#include "datapath.h"
#include "debug.h"
struct hw ixgbe_adapter __attribute__((aligned(64))) = {0};
static struct ixgbe_stats stats = {0};
static union ixgbe_adv_rx_desc ixgbe_adv_rx_desc __attribute__((aligned(64))) = {0};
static union ixgbe_adv_tx_desc ixgbe_adv_tx_desc __attribute__((aligned(64))) = {0};

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
      struct ethhdr *eth = (struct ethhdr *)pkt;
      /* Definitions below are useless when the debug flag is not set.
       * With compiler optimization is enabled, it can be optimized by the compiler,
       * but we also doesn't use it. A solution will be decided.
       * Probably make command will be splitted to development and release,
       * and optimizations will be used on release.
       */
      u8* h_source = eth->h_source;
      DPRINT("source mac address: %0x:%0x:%0x:%0x:%0x:%0x\n", h_source[0], h_source[1], h_source[2], h_source[3], h_source[4], h_source[5]);
      u8* d_source = eth->h_dest;
      DPRINT("destination mac address: %0x:%0x:%0x:%0x:%0x:%0x\n", d_source[0], d_source[1], d_source[2], d_source[3], d_source[4], d_source[5]);
      struct iphdr *ip = (struct iphdr *)(pkt + sizeof(struct ethhdr));
      stats.total_bytes_rx = stats.total_bytes_rx + ip->tot_len;
      DPRINT("irrelevant packets : %u\n", stats.irrelevant_packets);
      DPRINT("total bytes on Rx %u\n", stats.total_bytes_rx);
      u32 src_ip = __builtin_bswap32(ip->saddr); /* See little endian/big endian byte orders. */
      DPRINT("source ip addrress: %0x\n", src_ip);
      u32 dst_ip = __builtin_bswap32(ip->daddr);
      DPRINT("destination ip address: %0x\n",dst_ip);
      if(unlikely(stats.batch_manage_tail_counter >= stats.batch_manage_tail)){
        ixgbe_write_reg(&ixgbe_adapter, IXGBE_RDT, i);
        stats.batch_manage_tail_counter = 0;
      }
      /* PoC data path 
      */
      /* IPv6 may be added but not included for now */
      struct icmphdr *icmp = (struct icmphdr *)(pkt + sizeof(struct ethhdr) + ip->ihl * 4);
      bool processed = ping_reply(eth, ip, icmp, &stats, i, rx_ring, tx_ring);
      wmb();
      /* Reset Descriptor Done */
      rx_ring[i].wb.status_error &= ~IXGBE_RXD_STAT_DD;
      rx_ring[i].read.pkt_addr = (u64)ixgbe_adapter.rx_base_phy + (256 * 1024) + (i * 2048);
      rx_ring[i].read.hdr_addr = 0;
      wmb();
      i = IXGBE_BUFFER_ADVANCE(i, 1);
      if(unlikely(!processed)){
        continue;
      }
      if(unlikely(stats.batch_tx_counter >= stats.batch_tx_transmit)){
      ixgbe_write_reg(&ixgbe_adapter, IXGBE_TDT, i);
      stats.batch_tx_counter = 0;
      } else {
      stats.batch_tx_counter++;
      }
      DPRINT("tail : %u\n", ixgbe_read_reg(&ixgbe_adapter,IXGBE_TDT));
      DPRINT("head : %u\n", ixgbe_read_reg(&ixgbe_adapter,IXGBE_TDH));
      }
  }
}