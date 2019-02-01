/*
 * listener_parser.c
 *
 *  Created on: 11.04.2018
 *      Author: KarolTrzcinski
 */

#include "../mac/mac.h"
#include "../mac/carry.h"

#define LISTENER_CASE(str,FC) \
	case FC: str = #FC; break;

void listener_parse(mac_buf_t *buf) {
	static uint32_t time = 0;
	uint32_t now = PORT_TickHr();
	uint32_t dt = PORT_TickHrToUs(now - time);
	time = mac_port_buff_time();
	const char *descriptor = 0;
	FC_CARRY_s *pcarry = (FC_CARRY_s*)&buf->frame.data[0];
	char carry_code = pcarry->hops[(pcarry->verHopsNum & 0xF) * sizeof(dev_addr_t)];
	char code = buf->frame.data[0];
	int type = buf->frame.control[0] & FR_CR_TYPE_MASK;
	char prot;

	switch (type) {
		case FR_CR_BEACON:
			prot = 'B';
			break;
		case FR_CR_ACK:
			prot = 'A';
			break;
		case FR_CR_DATA:
			prot = 'D';
			break;
		case FR_CR_MAC:
			prot = 'M';
			break;
		default:
			prot = 'O';
	}

	if (code == FC_CARRY) {
		code = carry_code;
		prot = 'C';
	}

	switch (code) {
		LISTENER_CASE(descriptor, FC_BEACON)
		LISTENER_CASE(descriptor, FC_TURN_ON)
		LISTENER_CASE(descriptor, FC_TURN_OFF)
		LISTENER_CASE(descriptor, FC_DEV_ACCEPTED)
		LISTENER_CASE(descriptor, FC_CARRY)
		LISTENER_CASE(descriptor, FC_FU)
		LISTENER_CASE(descriptor, FC_VERSION_ASK)
		LISTENER_CASE(descriptor, FC_VERSION_RESP)
		LISTENER_CASE(descriptor, FC_STAT_ASK)
		LISTENER_CASE(descriptor, FC_STAT_RESP)
		LISTENER_CASE(descriptor, FC_SYNC_POLL)
		LISTENER_CASE(descriptor, FC_SYNC_RESP)
		LISTENER_CASE(descriptor, FC_SYNC_FIN)
		LISTENER_CASE(descriptor, FC_TOA_INIT)
		LISTENER_CASE(descriptor, FC_TOA_POLL)
		LISTENER_CASE(descriptor, FC_TOA_RESP)
		LISTENER_CASE(descriptor, FC_TOA_FIN)
		LISTENER_CASE(descriptor, FC_TOA_RES)
		LISTENER_CASE(descriptor, FC_TDOA_BEACON_TAG)
		LISTENER_CASE(descriptor, FC_TDOA_BEACON_TAG_INFO)
		default:
			descriptor = "OTHER";
	}

	LOG_DBG("%4X:>%4X %3d %8d %2X %c %s", buf->frame.src, buf->frame.dst, buf->frame.seq_num, dt, code,
	        prot, descriptor);
}

void listener_isr(const dwt_cb_data_t *data) {
	LOG_Trace(TRACE_DW_IRQ_RX);
	IASSERT(settings.mac.role == RTLS_LISTENER);
	PORT_LedOn(LED_STAT);
	mac_buf_t *buf = MAC_Buffer();

	if (buf != 0) {
		TRANSCEIVER_Read(buf->buf, data->datalength);
		TRANSCEIVER_DefaultRx();

		buf->rx_len = data->datalength;
		listener_parse(buf);

		MAC_Free(buf);
	}
}
