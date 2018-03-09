#include "prot_parser.h"

typedef struct
{
    uint8_t FC;
    prot_parser_cb cb;
} prot_cb_t;

const prot_cb_t prot_cb_tab[] = {
    {FC_CARRY, 0},
};
const int prot_cb_len = sizeof(prot_cb_tab) / sizeof(*prot_cb_tab);

void prot_parse(mac_buf_t *buf, const prot_packet_info_t *info)
{
    IASSERT(buf != 0);
    unit8_t FC = *buf->dPtr;
    prot_cb_t *pcb;

    for (int i = 0, pcb = prot_cb_len; i < prot_cb_len; ++i, ++pcb)
    {
        if (FC == pcb->FC)
        {
            IASSERT(pcb->cb != 0);
            pcb->cb(buf->dPtr, info);
        }
    }
}