#ifndef DATAPATH_H
#define DATAPATH_H
#include "base.h"
#include "debug.h"
#include "ixgbe.h"
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
typedef enum {
    PROTO_UNKNOWN = 0,
    PROTO_ICMP    = IPPROTO_ICMP,
    PROTO_TCP     = IPPROTO_TCP,
    PROTO_UDP     = IPPROTO_UDP,
    PROTO_UNSUPPORTED = 254
} datapath_proto_t;
struct pkt_ctx {
  u8                 *pkt;
  struct ethhdr      *eth;
  struct iphdr       *ip;
  struct ixgbe_stats *stats;
  u32                 len;
  datapath_proto_t    proto;
};
static inline bool ping_reply(struct pkt_ctx* ctx,int i,union ixgbe_adv_rx_desc* rx_ring, union ixgbe_adv_tx_desc* tx_ring){
     if(unlikely(ctx->ip->version != 4)){
        rx_ring[i].wb.status_error = 0;
        ctx->stats->irrelevant_packets++;
        return false;
      }
      if(unlikely((ctx->ip->protocol != 1))){
        rx_ring[i].wb.status_error = 0;
        ctx->stats->irrelevant_packets++;
        return false;
      }
      /* After the checks, there's no branch before transmitting. */
      ctx->stats->total_bytes_tx = ctx->stats->total_bytes_tx + ctx->ip->tot_len;
      /* Swap MAC's */
      for (int j = 0; j < 6; j++) {
      u8 tmp = ctx->eth->h_source[j];
      ctx->eth->h_source[j] = ctx->eth->h_dest[j];
      ctx->eth->h_dest[j] = tmp;
      }
      /* Swap IP's */
      u32 tmp_ip = ctx->ip->daddr;
      ctx->ip->daddr = ctx->ip->saddr;
      ctx->ip->saddr = tmp_ip;
      /* Change type to reply on ICMP */
      struct icmphdr *icmp = (struct icmphdr *)(ctx->pkt + sizeof(struct ethhdr) + ctx->ip->ihl * 4);
      if(likely(icmp->type == 8)){
        icmp->type = 0;
      /* Calculate new checksum with RFC 1624 */
      u32 chk_new = ntohs(icmp->checksum) + 0x0800;
      if (unlikely(chk_new > 0xFFFF))
        chk_new = (chk_new & 0xFFFF) +1;
      icmp->checksum = htons(chk_new);
      }
      u64 transmit = ixgbe_adapter.rx_base_phy + (256 * 1024) + ( i * 2048);
      union ixgbe_adv_tx_desc *tx_desc = &tx_ring[i];
      tx_desc->data_read.address = transmit;
      tx_desc->data_read.dtalen = rx_ring[i].wb.length;
      tx_desc->data_read.paylen = rx_ring[i].wb.length;
      tx_desc->data_read.dtyp = 3;
      tx_desc->data_read.rs = 1;
      tx_desc->data_read.eop = 1;
      tx_desc->data_read.ifcs = 1;
      tx_desc->data_read.dext = 1;
      tx_desc->data_read.dd = 0;
      return true;
}
#endif