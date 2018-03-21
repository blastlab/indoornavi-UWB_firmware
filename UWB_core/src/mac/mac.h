#ifndef _MAC_H
#define _MAC_H
#include "../logs.h"
#include "../prot/prot_const.h"
#include "../transceiver.h"


#include "mac_const.h"
#include "mac_port.h"
#include "mac_settings.h"
#include "sync.h"


typedef struct {
  union {
    unsigned char buf[MAC_BUF_LEN];
    struct _packed {
      unsigned char control;
      unsigned char seq_num;
      pan_dev_addr_t pan;
      dev_addr_t src;
      dev_addr_t dst;
      // unsigned int time;
      unsigned char data[64];
    } frame;
  };
  unsigned char *dPtr;
  mac_buf_state state;
  unsigned char isRangingFrame;
  short retransmit_fail_cnt;
  unsigned int last_update_time;
} mac_buf_t;

typedef struct {
  int slot_number;
  mac_buf_t buf[MAC_BUF_CNT];
  mac_buf_t rx_buf;
  short buf_get_ind;
  unsigned int sync_offset;
  mac_buf_t *buf_under_tx;
} mac_instance_t;

// initialize mac and transceiver
void mac_init();

// should be called at the beginning of your time slot
void mac_transmit_buffers();

// should be called from the frame transmitted isr
// @param is last frame tx timestamp in dw unit time
int mac_transmitted_isr(int64_t tx_timestamp);

// should be called at the beginning of your slot time
void mac_your_slot_isr();

// release buffer waiting for this ack
void mac_ack_frame_isr(uint8_t seq_num);

// reserve buffer
mac_buf_t *mac_buffer();

// return length of already written frame
int mac_buf_len(const mac_buf_t *buf);

// reserve buffer and fill mac protocol fields
// @param address to target device in range of radio - without hops
// @param true if it can be appended to some other packet to this target
mac_buf_t *mac_buffer_prepare(dev_addr_t target, bool can_append);

// free buffer
// @param pointer to mac_but_t
void mac_free(mac_buf_t *buf);

// add frame to the transmit queue, buf will be released after transmission
void mac_send(mac_buf_t *buf, bool ack_require);

// change dst and src address, send according to flags, buf will be released
// after transmission
int mac_send_ranging_resp(mac_buf_t *buf, uint8_t transceiver_flags);

unsigned char mac_read8(mac_buf_t *frame);
void mac_write8(mac_buf_t *frame, unsigned char value);
void mac_read(mac_buf_t *frame, void *destination, unsigned int len);
void mac_write(mac_buf_t *frame, const void *src, unsigned int len);

#endif // _MAC_H
