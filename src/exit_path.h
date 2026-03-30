#include <stdio.h>
#include <unistd.h>

#include "base.h"
#include "hw.h"
#include "ixgbe.h"

static inline void exit_entrypoint(struct ixgbe_stats* __restrict stats,
                                   struct hw* __restrict hw) {
  printf("stats from sw : \n");
  printf("batch_manage_tail : %u\n", stats->batch_manage_tail);
  printf("batch_manage_tail_counter : %u\n", stats->batch_manage_tail_counter);
  printf("total packets : %u\n", stats->total_packets);
  printf("irrelevant packets : %u\n", stats->irrelevant_packets);
  printf("irrelevant messages : %u\n", stats->irrelevant_messages);
  printf("batch tx counter : %u\n", stats->batch_tx_counter);
  printf("total bytes rx : %u\n", stats->total_bytes_rx);
  printf("total bytes tx : %u\n", stats->total_bytes_tx);
  printf("batch tx_transmit : %u\n", stats->batch_tx_transmit);
  printf("ring full_drop : %u\n", stats->ring_full_drop);
  printf("stats from fw :\n");
  u32 rx_tail = ixgbe_read_reg(hw, IXGBE_RDT);
  u32 rx_head = ixgbe_read_reg(hw, IXGBE_RDH);
  u32 tx_tail = ixgbe_read_reg(hw, IXGBE_TDT);
  u32 tx_head = ixgbe_read_reg(hw, IXGBE_TDH);
  /*
   * According to errata 57, spec update rev 4.3.3,
   * some registers, including RXMPC, might retieve an incorrect ue when
   * performaing back-to-back read.
   */
  u32 mpc = ixgbe_read_reg(hw, IXGBE_RXMPC);
  printf(" Missed packets count : %u\n", mpc);
  usleep(1);
  /*
   * According to errata 7, spec update rev 4.3.3,
   * GPRC and GORCL/H counters incorrectly include missed packets/bytestats->
   * Workaround implemented.
   */
  u32 gprc = ixgbe_read_reg(hw, IXGBE_GPRC);
  u32 remove = ixgbe_read_reg(hw, IXGBE_RXMPC);
  gprc -= remove;
  printf(" Good packets received : %u\n", gprc);
}