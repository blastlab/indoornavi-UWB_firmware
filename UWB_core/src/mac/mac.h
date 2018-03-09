#ifndef _MAC_H
#define _MAC_H
#include "logs.h"
#include "transceiver.h"
#include "prot/prot_const.h"

#include "mac/mac_const.h"
#include "mac/mac_settings.h"
#include "mac/mac_port.h"

typedef struct
{
    union {
        unsigned char buf[MAC_BUF_LEN];
        struct _packed
        {
            unsigned char control;
            unsigned char seq_num;
            pan_dev_addr_t pan;
            dev_addr_t src;
            dev_addr_t dst;
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
    dev_addr_t addr;
    pan_dev_addr_t pan;
    mac_buf_t buf[MAC_BUF_CNT];
    short buf_get_ind;
    unsigned int sync_offset;
} mac_instance_t;

// should be called at the beginning of your time slot
void mac_transmit_buffers();

// should be called from the frame transmitted isr
// @param is last frame tx timestamp in dw unit time
int mac_transmitted_isr(uint64_t tx_timestamp);

// should be called at the beginning of your slot time
void mac_your_slot_isr();

// release buffer waiting for this ack
void mac_ack_frame_isr(uint8_t seq_num);

// reserve buffer
mac_buf_t *mac_buffer();

// reserve buffer and fill mac protocol fields
// @param address to target device in range of radio - without hops
// @param true if it can be appended to some other packet to this target
mac_buf_t *mac_buffer_prepare(dev_addr_t target, bool can_append);

// free buffer
// @param pointer to mac_but_t
void mac_free(mac_buf_t *buf);

// add frame to the transmit queue, buf will be released after transmission
void mac_send(mac_buf_t *buf, bool ack_require);

unsigned char mac_read8(mac_buf_t *frame);
void mac_write8(mac_buf_t *frame, unsigned char value);
void mac_read(mac_buf_t *frame, void *destination, unsigned int len);
void mac_write(mac_buf_t *frame, const void *src, unsigned int len);

#endif // _MAC_H
