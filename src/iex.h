#ifndef IEX_H
#define IEX_H
#include <asm-generic/errno-base.h>
#include <stdio.h>

#include "base.h"
#define IEX_TRADING_STATUS 0x48
#define IEX_SSPTSM 0x50 // Short Sale Price Test Status Message
#define IEX_QUM_MTYPE 0x51  // IEX Quote Update Message Message type.
#define IEX_SYSTEM_EVENT 0x53
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
struct __attribute__((packed)) IEX_Trading_Status {
  u8 message_type;
  u8 IEX_TradingStatus_t;
  u64 timestamp;
  char symbol[8];
  char reason[4];
};
typedef char IEX_TradingStatus_t;
#define IEX_STATUS_HALT 'H'
#define IEX_STATUS_ORDER_ACCEPTANCE 'O'
#define IEX_STATUS_PAUSE 'P'
#define IEX_STATUS_ACTIVE 'T'
struct __attribute__((packed)) IEX_System_Event {
  u8 message_type;
  u8 IEX_System_Event_t;
  u64 timestamp;
};
typedef char IEX_System_Event_t;
#define IEX_START_OF_MESSAGES 'O'
#define IEX_START_OF_SYSTEM_HOURS 'S'
#define IEX_START_OF_REGULAR_MARKET_HOURS 'R'
#define IEX_END_OF_REGULAR_MARKET_HOURS  'M'
#define IEX_END_OF_SYSTEM_HOURS 'E'
#define IEX_END_OF_MESSAGES 'C'
struct __attribute__((packed)) IEX_Short_Sale_Price_Test {
  u8 message_type;
  u8 status;
  u64 timestamp;
  char symbol[8];
  u8 detail;
};
static inline int iex_test_ds() {
  if (unlikely(sizeof(struct IEX_Quote_Update) != 42)) {
    printf(
        "size of the IEX quote update data structure doesn't matches "
        "documentation. %zu\n",
        sizeof(struct IEX_Quote_Update));
        return -EINVAL;
  }
  if (unlikely(sizeof(struct IEX_TP_header) != 40)) {
    printf(
        "size of the IEX Transportation header data structure doesn't matches "
        "documentation. %zu\n",
        sizeof(struct IEX_TP_header));
        return -EINVAL;
  }
  if (unlikely(sizeof(struct IEX_Trading_Status) != 22)){
    printf(
        "size of the IEX Trading status data structure doesn't matches "
        "documentation. %zu\n",
        sizeof(struct IEX_Trading_Status));
        return -EINVAL;
  }
  if (unlikely(sizeof(struct IEX_System_Event) != 10)){
    printf(
        "size of the IEX System Event data structure doesn't matches "
        "documentation. %zu\n",
        sizeof(struct IEX_System_Event));
        return -EINVAL;
  }
  if (unlikely(sizeof (struct IEX_Short_Sale_Price_Test) != 19)){
    printf(
        "size of the IEX Short Sale Price data structure doesn't matches "
        "documentation. %zu\n",
        sizeof(struct IEX_Short_Sale_Price_Test));
        return -EINVAL;
  }
  return 0;
}
#endif  // IEX_H