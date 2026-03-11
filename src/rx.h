#ifndef RX_H
#define RX_H
#include "base.h"
#include "datapath.h"
#include "hw.h"
#include "ixgbe.h"
#include <netinet/ip.h>
static inline u8 rx_entry(struct pkt_ctx* ctx,int i,union ixgbe_adv_rx_desc* rx_ring){
  ctx->len = rx_ring[i].wb.length;
  if(unlikely(ctx->len < sizeof(struct ethhdr))) return PROTO_UNSUPPORTED;
  ctx->eth =(struct ethhdr *)(ctx->pkt);
  ctx->ip = (struct iphdr *)(ctx->pkt + sizeof(struct ethhdr));
  if(unlikely(ctx->len <  sizeof(struct ethhdr) + sizeof(struct iphdr))) return PROTO_UNSUPPORTED;
  ctx->stats->total_bytes_rx = ctx->stats->total_bytes_rx + ctx->ip->tot_len;
  if(unlikely(ctx->stats->batch_manage_tail_counter >= ctx->stats->batch_manage_tail)){
        ixgbe_write_reg(&ixgbe_adapter, IXGBE_RDT, i);
        ctx->stats->batch_manage_tail_counter = 0;
      }  
  return ctx->ip->protocol;
}
#endif