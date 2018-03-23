#include "bin_parser.h"

void bin_parse(mac_buf_t *buf, const prot_packet_info_t *info)
{
    IASSERT(buf != 0);
    uint8_t FC = *buf->dPtr;
    const prot_cb_t *pcb = &prot_cb_tab[0];

    for (int i = 0; i < prot_cb_len; ++i, ++pcb)
    {
        if (FC == pcb->FC)
        {
            IASSERT(pcb->cb != 0);
            pcb->cb(buf->dPtr, info);
        }
    }
}