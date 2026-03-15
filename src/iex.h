#ifndef IEX_H
#define IEX_H
#include "base.h"
struct __attribute__((packed)) IEX_Quote_Update {
        u8  message_type; 
        u8  flags;        
        u64 timestamp;  
        char     symbol[8];     
        u64 bid_price;     
        u64 ask_price;     
        u32 ask_size;      
};
#endif //IEX_H