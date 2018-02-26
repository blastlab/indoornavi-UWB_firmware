#ifndef _MAC_CONST
#define _MAC_CONST

typedef enum {
    FREE,
    BUSY,
    WAIT_FOR_TX,
    WAIT_FOR_TX_ACK,
    WAIT_FOR_ACK,
} mac_buf_state;

#endif