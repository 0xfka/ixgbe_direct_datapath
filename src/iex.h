#ifndef IEX_H
#define IEX_H
#include <asm-generic/errno-base.h>
#include <stdio.h>

#include "base.h"
#define IEX_QUM_MTYPE 0x51  // IEX Quote Update Message Message type.
struct __attribute__((packed)) IEX_Quote_Update {
  u8 message_type;
  u8 flags;
  u64 timestamp;
  char symbol[8];
  u32 bid_size;
  u64 bid_price;
  u64 ask_price;
  u32 ask_size;
};
struct __attribute__((packed)) IEX_TP_header {
  u8 version;
  u8 reserved;
  u16 message_proto_id;
  u32 channel_id;
  u32 session_id;
  u16 payload_length;
  u16 mesg_count;
  u64 stream_offset;
  u64 first_mesg_seq_num;
  u64 send_time;
};
inline int iex_test_ds() {
  if (unlikely(sizeof(struct IEX_Quote_Update) != 42)) {
    printf(
        "size of the IEX quote update data structure doesn't matches "
        "documentation. %lu\n",
        sizeof(struct IEX_Quote_Update));
        return -EINVAL;
  }
  if (unlikely(sizeof(struct IEX_TP_header) != 40)) {
    printf(
        "size of the IEX Transportation header data structure doesn't matches "
        "documentation. %lu\n",
        sizeof(struct IEX_TP_header));
        return -EINVAL;
  }
  return 0;
}
#endif  // IEX_H