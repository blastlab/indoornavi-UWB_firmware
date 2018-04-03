#ifndef _BIN_PARSER_H
#define _BIN_PARSER_H

#include "../iassert.h"
#include "../mac/mac.h"
#include "../mac/mac_const.h"

#define BIN_ASSERT(expr) IASSERT(expr)

typedef void (*prot_parser_cb)(const void *data,
                               const prot_packet_info_t *info);

typedef struct {
  uint8_t FC;
  prot_parser_cb cb;
} prot_cb_t;

uint8_t BIN_ParseSingle(mac_buf_t *buf, const prot_packet_info_t *info);
void BIN_Parse(mac_buf_t *buf, const prot_packet_info_t *info, uint8_t size);
void BIN_TxCb(int64_t Ts);
void BIN_RxTimeoutCb();

#endif // _PROT_PARSER_H
