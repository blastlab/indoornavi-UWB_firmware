#include "bin_parser.h"

uint8_t BIN_ParseSingle(const uint8_t *buf, const prot_packet_info_t *info) {
  IASSERT(buf != 0);
  extern const prot_cb_t prot_cb_tab[];
  extern const int prot_cb_len;
  uint8_t FC = buf[0];
  const prot_cb_t *pcb = &prot_cb_tab[0];

  for (int i = 0; i < prot_cb_len; ++i, ++pcb) {
    if (FC == pcb->FC) {
      IASSERT(pcb->cb != 0);
      pcb->cb(buf, info);
      return buf[1]; // frame len
    }
  }
  return 0xFF;
}

void BIN_Parse(mac_buf_t *buf, const prot_packet_info_t *info, int size) {
  uint8_t ret = 0;
  uint8_t *t_dPtr = buf->dPtr;

  while (size > 0) {
    ret = BIN_ParseSingle(buf, info);
    if(ret == 0xFF) {
      LOG_ERR("unknown opcode %X", *buf->dPtr);
      ret = buf->dPtr[1]; // frame len
    }
		size -= ret;
		t_dPtr += ret;
		// override buf->dPtr in case of bad function code callback
		buf->dPtr = t_dPtr;
  }
}
