#ifndef _MAC_H
#define _MAC_H
#include "../parsers/bin_const.h"
#include "logs.h"
#include "transceiver.h"

#include "mac/mac_const.h"
#include "mac/mac_port.h"
#include "mac/mac_settings.h"
#include "mac/sync.h"

// macro to find unreleased buffer at compilation time
#define MAC_USAGE_BUF_START(name)                                              \
  mac_buf_t *#name = MAC_Buffer();                                             \
  if (#name != 0) {

#define MAC_USAGE_BUF_STOP(name)                                               \
  MAC_Free(#name);                                                             \
  }

#define MAC_HEAD_LENGTH (2+1+sizeof(pan_dev_addr_t)+2*sizeof(dev_addr_t))

typedef struct {
  union {
    unsigned char buf[MAC_BUF_LEN];
    struct _packed {
      unsigned char control[2];
      unsigned char seq_num;
      pan_dev_addr_t pan;
      dev_addr_t dst;
      dev_addr_t src;
      // unsigned int time;
      unsigned char data[64];
    } frame;
  };
  unsigned char *dPtr;
  mac_buf_state state;
  int rx_len;
  unsigned char isRangingFrame;
  short retransmit_fail_cnt;
  unsigned int last_update_time;
} mac_buf_t;

typedef struct {
  int slot_number;
  mac_buf_t buf[MAC_BUF_CNT];
  uint8_t seq_num;
  mac_buf_t rx_buf;
  short buf_get_ind;
  int64_t slot_time_offset;
  mac_buf_t *buf_under_tx;
  unsigned int last_rx_ts;
} mac_instance_t;


// used by mac, externally implemented
void listener_isr(const dwt_cb_data_t *data);

// initialize mac and transceiver
void MAC_Init();

// should be called at the beginning of your slot time
void MAC_YourSlotIsr();

// release buffer waiting for this ack
void MAC_AckFrameIsr(uint8_t seq_num);

// reserve buffer
mac_buf_t *MAC_Buffer();

// return length of already written frame
int MAC_BufLen(const mac_buf_t *buf);

// low level function, used only by carry module
void MAC_FillFrameTo(mac_buf_t *buf, dev_addr_t target);

// reserve buffer and fill mac protocol fields
// @param address to target device in range of radio - without hops
// @param true if it can be appended to some other packet to this target
mac_buf_t *MAC_BufferPrepare(dev_addr_t target, bool can_append);

// free buffer
// @param pointer to mac_but_t
void MAC_Free(mac_buf_t *buf);

// add frame to the transmit queue, buf will be released after transmission
void MAC_Send(mac_buf_t *buf, bool ack_require);

// change dst and src address, send according to flags, buf will be released
// after transmission
int MAC_SendRanging(mac_buf_t *buf, uint8_t transceiver_flags);

// return time in ms from last received packed
unsigned int MAC_UsFromRx();

unsigned char MAC_Read8(mac_buf_t *frame);
void MAC_Write8(mac_buf_t *frame, unsigned char value);
void MAC_Read(mac_buf_t *frame, void *destination, unsigned int len);
void MAC_Write(mac_buf_t *frame, const void *src, unsigned int len);

#endif // _MAC_H
