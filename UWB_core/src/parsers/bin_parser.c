#include "bin_parser.h"

void bin_parse(mac_buf_t *buf, const prot_packet_info_t *info)
{
    IASSERT(buf != 0);
    unit8_t FC = *buf->dPtr;
    prot_cb_t *pcb;

    for (int i = 0, pcb = &prot_cb_tab[0]; i < prot_cb_len; ++i, ++pcb)
    {
        if (FC == pcb->FC)
        {
            IASSERT(pcb->cb != 0);
            pcb->cb(buf->dPtr, info);
        }
    }
}