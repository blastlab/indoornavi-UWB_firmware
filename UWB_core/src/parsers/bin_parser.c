#include "bin_parser.h"

uint8_t BIN_ParseSingle(mac_buf_t *buf, const prot_packet_info_t *info)
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
            return buf->dPtr[1]; // frame len
        }
    }
    return 0xFF;
}


void BIN_Parse(mac_buf_t *buf, const prot_packet_info_t *info, uint8_t size)
{
	uint8_t ret = 0;
	uint8_t *s_dPtr = buf->dPtr;
	uint8_t *t_dPtr = buf->dPtr;

	while(size > 0)
	{
		ret = BIN_ParseSingle(buf, info);
		if(ret < 0xFF)
		{
			size -= ret;
			t_dPtr += ret;
			buf->dPtr = t_dPtr;
		}
		else
		{
			LOG_ERR("unknown opcode %X", *buf->dPtr);
			buf->dPtr = s_dPtr + size;
		}
	}

}
