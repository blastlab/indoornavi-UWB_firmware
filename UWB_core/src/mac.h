#ifndef _MAC_H
#define _MAC_H
#include "prot_const.h"
#include "transceiver.h"
#include "mac_const.h"
#include "mac_settings.h"
#include "mac_port.h"

typedef struct
{
    union {
        unsigned char buf[MAC_BUF_LEN];
        struct _packed
        {
            unsigned char seq_num;
            addr_t src;
            addr_t dst;
            //unsigned int time;
            unsigned char data[64];
        } frame;
    };
    unsigned char *dPtr;
    mac_buf_state state;
    short retransmit_fail_cnt;
    unsigned int last_update_time;
} mac_buf_t;

typedef struct
{
    addr_t addr;
    mac_buf_t buf[MAC_BUF_CNT];
    short buf_get_ind;
} mac_instance_t;

// should be called at the beginning of your time slot
void mac_transmit_buffers();

// should be called from the frame transmitted isr
int mac_transmitted_isr()

    // reserve buffer
    mac_buf_t *mac_buffer();

// reserve buffer and fill mac protocol fields
mac_buf_t *mac_buffer_prepare(addr_t target, bool can_append);

// free buffer
void mac_free(mac_buf_t *);

// add frame to the transmit queue, buf will be released after transmission
void mac_send(const mac_buf_t *buf);

unsigned char mac_read8(mac_buf_t *frame);
void mac_write8(mac_buf_t *frame, unsigned char value);
void mac_read(mac_buf_t *frame, void *destination, unsigned int len);
void mac_write(mac_buf_t *frame, const void *src, unsigned int len);

#endif // _MAC_H
