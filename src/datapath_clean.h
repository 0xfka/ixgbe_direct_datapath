#ifndef DATAPATH_CLEAN_H
#define DATAPATH_CLEAN_H
#include "datapath.h"
#include "ixgbe.h"
static inline void datapath_clean(struct pkt_ctx* pkt_ctx, bool is_processed,int i, union ixgbe_adv_rx_desc* rx_ring, union ixgbe_adv_tx_desc* tx_ring){
  rx_ring[i].wb.status_error &= ~IXGBE_RXD_STAT_DD;
  rx_ring[i].read.pkt_addr = (u64)ixgbe_adapter.rx_base_phy + (256 * 1024) + (i * 2048);
  rx_ring[i].read.hdr_addr = 0;
  wmb();
  if(unlikely(!is_processed)){
   return;
}
  if(unlikely(pkt_ctx->stats->batch_tx_counter >= pkt_ctx->stats->batch_tx_transmit)){
  ixgbe_write_reg(&ixgbe_adapter, IXGBE_TDT, i);
  pkt_ctx->stats->batch_tx_counter = 0;
} else {
  pkt_ctx->stats->batch_tx_counter++;
}
  return;
}
#endif