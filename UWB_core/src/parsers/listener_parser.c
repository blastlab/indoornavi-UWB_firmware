/*
 * listener_parser.c
 *
 *  Created on: 11.04.2018
 *      Author: KarolTrzcinski
 */


#include "../mac/mac.h"

#define LISTENER_CASE(str,FC) \
	case FC: str = #FC; break;


void listener_parse(mac_buf_t *buf) {
	static uint32_t time = 0;
	uint32_t now = PORT_TickHr();
	uint32_t dt = PORT_TickHrToUs(now - time);
	time = mac_port_buff_time();
	const char *descriptor = 0;

	switch(buf->frame.data[0]) {
	LISTENER_CASE(descriptor, FC_CARRY);
	LISTENER_CASE(descriptor, FC_BEACON);
	LISTENER_CASE(descriptor, FC_SYNC_POLL);
	LISTENER_CASE(descriptor, FC_SYNC_RESP);
	LISTENER_CASE(descriptor, FC_SYNC_FIN);
	default:
		descriptor = "OTHER";
	}

	LOG_INF("%X-%X %3d %8d %2X %s", buf->frame.src, buf->frame.dst, buf->frame.seq_num, dt, buf->frame.data[0], descriptor);
}


void listener_isr(const dwt_cb_data_t *data) {
  PORT_LedOn(LED_STAT);
  mac_buf_t *buf = MAC_Buffer();

  if (buf != 0) {
    TRANSCEIVER_Read(buf->buf, data->datalength);
    TRANSCEIVER_DefaultRx();

    buf->rx_len = data->datalength;

    if(settings.mac.role == RTLS_LISTENER) {
    	listener_parse(buf);
    } else {
    	MAC_Init();
    }

    MAC_Free(buf);
  } else {
    LOG_ERR("No buff for rx_cb");
  }
}
