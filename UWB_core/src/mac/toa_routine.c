/**
 * @brief Time Of Arrival routines
 *
 * @file toa_routine.c
 * @author Karol Trzcinski
 * @date 2018-07-20
 *
 * Firsly, from sink send Init or Poll message.
 *   a) poll is sent if sink is a measurement target (to anchors from measurement list)
 *   b) otherwise send Init message via Carry protocol
 * (It is done by ranging module).
 * Then in Init send routine we need to distinguish two situations:
 *   a) target is TAG, then send Init to main anchor in poll (first in the poll anchor array),
 *      add target TAG address after anchors array, increment anchors counter
 *   b) target isn't TAG, then send Init to target anchor
 * Save ranging info in toa.core
 *
 * Init callback:
 *   set your mac parrent, then
 *   a) you are a main anchor (first in the poll anchor array), then remove TAG address from anchors
 *      array and decrement anchors array counter, send Init to target TAG
 *   b) otherwise you are a measurement target so send Poll message (to anchors from measurement
 *      list)
 *
 * Send Poll routine:
 *   Save ranging info in toa.core
 *   Send MAC message directly on via broadcast message (when is more than 1 anchor to measure)
 *
 * Poll callback:
 *   Save ranging info in toa.core: core.TsPollRx (40b), core.initiator, core.anc_in_poll_cnt,
 *   anchors array.
 *   Zero core.resp_ind, set your core.resp_ind
 *   It it was poll to you, then send RESP in a predicted tx time
 *
 * Send Poll completed interrupt:
 *   Save PollTxTs (40b) in toa.core
 *
 * Send Resp:
 *   Calculate tx time (from input PollDwRxTs) and save to core.TsRespTx and transceiver
 *   Send response to core.initiator (core.TsPollRx and core.TsRespTx is in packet)
 *   Calculate tx_to_rx_dly_us to turn on receiver just before FIN message
 *   Set rx timeout to 1 guard time after start of transmitting FIN message, base on DwPollRxTs
 *
 * Send Resp completed interrupt:
 *   Save RespTxTs (40b) in toa.core
 *
 * Resp callback:
 *   Save rx ts to core.TsRespRx[toa.core.resp_ind++]
 *   if you received all RESP then send FIN
 *   else enable rx just before ext RESP or FIN
 *
 * Receive Resp Time out:
 *   increment toa.core.resp_int
 *   if it is greater or equal to toa.core.anc_in_poll_cnt then send final
 *   else forcetrxoff, turn on receiver, TOA_State(TOA_IDLE)
 *
 * Send Fin completed interrupt:
 *   Save FinTxTs (40b) in toa.core
 *   Turn on receiver
 *
 * Fin callback:
 *   Save toa.core.TsFinTx, toa.core.TsPollTx, toa.core.TsFinRx
 *   calculate distance and push it to database
 *   forcetrxoff, turn on receiver
 *
 */

#include "toa_routine.h"
#include "carry.h"
#include "mac.h"

extern toa_instance_t toa;
extern mac_instance_t mac;

#define PROT_CHECK_LEN(FC, len, expected)               \
  do {                                                  \
    if ((len) < (expected)) {                           \
      LOG_ERR(ERR_BAD_OPCODE_LEN, #FC, (len), (expected)); \
      return -1;                                        \
    }                                                   \
  } while (0)

void TOA_InitDly() {
	const int rx_to_tx_delay = 30;  // us
	const float spi_speed = 20e6;   // Hz
	const int POLL_PROCESSING_TIME_US = 270;
	const int RESP_PROCESSING_TIME_US = 200;

	toa_settings_t* tset = &settings.mac.toa_dly;

	if (tset->guard_time_us != 0) {
		return;
	}

	tset->guard_time_us = 50;  // us
	tset->rx_after_tx_offset_us = tset->guard_time_us;
	tset->rx_after_tx_offset_us += TRANSCEIVER_EstimateTxTimeUs(0);

	// add here 2 bytes for CRC
	int basePollLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_POLL_s);
	basePollLen += sizeof(dev_addr_t) * TOA_MAX_DEV_IN_POLL;
	const int respLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_RESP_s);
	const int baseFinLen = MAC_HEAD_LENGTH + sizeof(FC_SYNC_FIN_s);
	int len;

	for (int i = 0; i < TOA_MAX_DEV_IN_POLL; ++i) {
		len = basePollLen;
		tset->resp_dly_us[i] = POLL_PROCESSING_TIME_US;
		tset->resp_dly_us[i] += 1e6 * len / spi_speed;
		tset->resp_dly_us[i] += rx_to_tx_delay + tset->guard_time_us;
		tset->resp_dly_us[i] += TRANSCEIVER_EstimateTxTimeUs(len);
		tset->resp_dly_us[i] += i
		    * (TRANSCEIVER_EstimateTxTimeUs(respLen) + RESP_PROCESSING_TIME_US + tset->guard_time_us);
	}

	len = baseFinLen + TOA_MAX_DEV_IN_POLL * 5;
	tset->fin_dly_us = RESP_PROCESSING_TIME_US;
	tset->fin_dly_us += 1e6 * len / spi_speed;
	tset->fin_dly_us += rx_to_tx_delay + tset->guard_time_us;
	tset->fin_dly_us += TRANSCEIVER_EstimateTxTimeUs(respLen);

	tset->poll_frame_duration_us = TRANSCEIVER_EstimateTxTimeUs(basePollLen);
	tset->fin_frame_duration_us = TRANSCEIVER_EstimateTxTimeUs(baseFinLen);
}

int TOA_SendInit(dev_addr_t dst, const dev_addr_t anchors[], int anc_cnt) {
	// Init should be sent to dst device when it is anchor
	// otherwise to first anchor from array and tag address should be added
	// as a last anchor
	TOA_ASSERT(0 < anc_cnt && anc_cnt < TOA_MAX_DEV_IN_POLL);
	dev_addr_t tempTarget;
	if ((dst & ADDR_ANCHOR_FLAG) || anchors[0] == settings.mac.addr) {
		tempTarget = dst;
	} else {
		tempTarget = anchors[0];
	}
	FC_CARRY_s* carry;
	mac_buf_t* buf = CARRY_PrepareBufTo(tempTarget, &carry);
	int anc_addr_len = anc_cnt * sizeof(dev_addr_t);
	if (buf == 0) {
		return -1;
	}

	if (tempTarget != dst) {
		anc_cnt++;
	}
	FC_TOA_INIT_s packet = { .FC = FC_TOA_INIT, .len = sizeof(FC_TOA_INIT_s)
	    + anc_cnt * sizeof(dev_addr_t), .num_poll_anchor = anc_cnt, };
	CARRY_Write(carry, buf, &packet, sizeof(FC_TOA_INIT_s));
	if (tempTarget != dst) {
		CARRY_Write(carry, buf, anchors, sizeof(dev_addr_t) * (anc_cnt - 1));
		CARRY_Write(carry, buf, &dst, sizeof(dev_addr_t));
	} else {
		CARRY_Write(carry, buf, anchors, sizeof(dev_addr_t) * anc_cnt);
	}

	toa.core.resp_ind = 0;
	toa.core.anc_in_poll_cnt = anc_cnt;
	toa.core.initiator = settings.mac.addr;
	memcpy(&toa.core.addr_tab[0], anchors, anc_addr_len);

	CARRY_Send(buf, false);
	return 0;
}

int TOA_SendPoll(const dev_addr_t anchors[], int anc_cnt) {
	TOA_ASSERT(0 < anc_cnt && anc_cnt < TOA_MAX_DEV_IN_POLL);
	mac_buf_t* buf = MAC_BufferPrepare(anc_cnt == 1 ? anchors[0] : ADDR_BROADCAST, false);
	int anc_addr_len = anc_cnt * sizeof(dev_addr_t);
	if (buf == 0) {
		return -1;
	}

	FC_TOA_POLL_s packet = { .FC = FC_TOA_POLL, .len = sizeof(FC_TOA_POLL_s)
	    + anc_cnt * sizeof(dev_addr_t), .num_poll_anchor = anc_cnt, };
	MAC_Write(buf, &packet, sizeof(FC_TOA_POLL_s));
	MAC_Write(buf, anchors, sizeof(dev_addr_t) * anc_cnt);

	toa.core.resp_ind = 0;
	toa.core.anc_in_poll_cnt = anc_cnt;
	toa.core.initiator = settings.mac.addr;
	memcpy(&toa.core.addr_tab[0], anchors, anc_addr_len);

	// send this frame in your slot (through queue) but with ranging flage
	// mac module should set DWT_RESPONSE_EXPECTED flag before transmission
	buf->isRangingFrame = true;
	TOA_State(&toa.core, TOA_POLL_WAIT_TO_SEND);
	MAC_SetFrameType(buf, FR_CR_MAC);
	 MAC_Send(buf, false);
	return 0;
}

static int TOA_SendResp(int64_t PollDwRxTs) {
	toa_settings_t* tset = &settings.mac.toa_dly;
	int resp_dly_us = tset->resp_dly_us[toa.core.resp_ind];

	toa.core.TsRespTx = TOA_SetTxTime(PollDwRxTs, resp_dly_us);

	mac_buf_t* buf = MAC_BufferPrepare(toa.core.initiator, false);
	if (buf == 0) {
		TOA_State(&toa.core, TOA_IDLE);
		return 0;
	}
	FC_TOA_RESP_s packet = { .FC = FC_TOA_RESP, .len = sizeof(FC_TOA_RESP_s), .TsPollRx =
	    toa.core.TsPollRx, .TsRespTx = toa.core.TsRespTx, };
	MAC_Write(buf, &packet, packet.len);

	int tx_to_rx_dly_us = tset->resp_dly_us[toa.core.anc_in_poll_cnt - 1];
	tx_to_rx_dly_us += tset->fin_dly_us - resp_dly_us;  // == delay to tx fin
	tx_to_rx_dly_us -= tset->rx_after_tx_offset_us;     // substract preamble time
	int rx_time_us = tset->rx_after_tx_offset_us;		// receiver extra time before fin tx
	rx_time_us += settings.mac.toa_dly.guard_time_us + tset->fin_frame_duration_us;
	TOA_ASSERT(tx_to_rx_dly_us > 0);
	dwt_setrxaftertxdelay(tx_to_rx_dly_us);
	dwt_setrxtimeout(rx_time_us); // 1 is 1.0256 us

	TOA_State(&toa.core, TOA_RESP_WAIT_TO_SEND);
	const int tx_flags = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED;
	return MAC_SendRanging(buf, tx_flags);
}

static int TOA_SendFinal() {
	toa_settings_t* tset = &settings.mac.toa_dly;
	int fin_dly_us = tset->resp_dly_us[toa.core.anc_in_poll_cnt - 1];
	fin_dly_us += tset->fin_dly_us;
	int64_t TsFinTx = TOA_SetTxTime(toa.core.TsPollTx, fin_dly_us);

	mac_buf_t* buf = MAC_BufferPrepare(toa.core.addr_tab[0], false);
	FC_TOA_FIN_s packet = {
	    .FC = FC_TOA_FIN,
	    .len = sizeof(FC_TOA_FIN_s),
	    .slot_num = mac.slot_number,
	    .TsPollTx = toa.core.TsPollTx, };
	// calc global time in place to keep 40B resolution
	TOA_Write40bValue(&packet.TsFinTxBuf[0], TsFinTx);

	int resp_rx_ts_len = sizeof(*packet.TsRespRx) * toa.core.anc_in_poll_cnt;
	packet.len += resp_rx_ts_len;
	memcpy(&packet.TsRespRx[0], &toa.core.TsRespRx[0], resp_rx_ts_len);
	MAC_Write(buf, &packet, packet.len);

	TOA_State(&toa.core, TOA_FIN_WAIT_TO_SEND);
	const int tx_flags = DWT_START_TX_DELAYED;
	return MAC_SendRanging(buf, tx_flags);
}

int TOA_SendRes(const measure_t* measure) {
	FC_TOA_RES_s packet = { .FC = FC_TOA_RES, .len = sizeof(FC_TOA_RES_s), .meas = *measure };
	FC_CARRY_s* carry;
	mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &carry);
	if (buf != 0) {
		CARRY_Write(carry, buf, &packet, packet.len);
		CARRY_Send(buf, false);
	}
	return 0;
}

void FC_TOA_INIT_cb(const void* data, const prot_packet_info_t* info) {
	FC_TOA_INIT_s* packet = (FC_TOA_INIT_s*)data;
	TOA_ASSERT(packet->FC == FC_TOA_INIT);
	int expected_size = sizeof(*packet) + packet->num_poll_anchor * sizeof(dev_addr_t);
	if (packet->len != expected_size) {
		LOG_ERR(ERR_BAD_OPCODE_LEN, "FC_TOA_INIT", packet->len, expected_size);
		return;
	}

	CARRY_SetYourParent(info->last_src);

	if (packet->poll_addr[0] == settings.mac.addr) {
		// Tag is the target and you should send Init
		int ancCnt = packet->num_poll_anchor - 1;
		TOA_SendInit(packet->poll_addr[ancCnt], packet->poll_addr, ancCnt);
	} else {
		// You are the target and you should send Poll
		// Poll will be sent as MAC frame in your slot
		// so next frame in slot won't be sent until rx timeout,
		// error occurrence or success ranging result
		TOA_SendPoll(packet->poll_addr, packet->num_poll_anchor);
	}
}

static int FC_TOA_POLL_cb(const void* data, const prot_packet_info_t* info) {
	TOA_State(&toa.core, TOA_POLL_REC);
	FC_TOA_POLL_s* packet = (FC_TOA_POLL_s*)data;
	TOA_ASSERT(packet->FC == FC_TOA_POLL);
	PROT_CHECK_LEN(FC_TOA_POLL, packet->len, sizeof(FC_TOA_POLL_s));
	int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

	toa.core.TsPollRx = rx_ts;
	toa.core.initiator = info->original_src;
	toa.core.anc_in_poll_cnt = packet->num_poll_anchor;
	toa.core.resp_ind = 0;
	const int addr_len = sizeof(dev_addr_t) * packet->num_poll_anchor;
	memcpy(toa.core.addr_tab, packet->poll_addr, addr_len);

	// check if it is full sync to you
	toa.core.resp_ind = TOA_FindAddrIndexInResp(&toa.core, settings.mac.addr);
	if (toa.core.resp_ind < toa.core.anc_in_poll_cnt) {
		int ret = TOA_SendResp(rx_ts);
		if (ret != 0) {
			LOG_DBG("TOA POLL->RESP tx timeout (%d)", MAC_UsFromRx());
			TRANSCEIVER_DefaultRx();
		} else {
			int dly = settings.mac.toa_dly.resp_dly_us[toa.core.resp_ind];
			int lag = MAC_UsFromRx();
			int tx_time = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_TOA_RESP_s));
			TOA_TRACE("TOA RESP sent to %X (%d>%d+%d)", toa.core.initiator, dly, lag, tx_time);
		}
	} else {
		// indicate that last poll wasn't to you
		toa.core.resp_ind = TOA_MAX_DEV_IN_POLL;
	}
	return 0;
}

static int FC_TOA_RESP_cb(const void* data, const prot_packet_info_t* info) {
	TOA_State(&toa.core, TOA_RESP_REC);
	FC_TOA_RESP_s* packet = (FC_TOA_RESP_s*)data;
	TOA_ASSERT(packet->FC == FC_TOA_RESP);
	PROT_CHECK_LEN(FC_TOA_RESP, packet->len, sizeof(FC_TOA_RESP_s));
	int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

	if (toa.core.resp_ind >= toa.core.anc_in_poll_cnt) {
		// to jest sytuacja patologiczna
		// nie powina miec miejsca
		dwt_forcetrxoff();
		TRANSCEIVER_DefaultRx();
		return 0;
	} else {
		toa.core.TsRespRx[toa.core.resp_ind++] = rx_ts;
	}
	if (toa.core.resp_ind >= toa.core.anc_in_poll_cnt) {
		int ret = TOA_SendFinal();
		if (ret != 0) {
			TOA_TRACE("TOA RESP->FIN tx timeout (%d)", MAC_UsFromRx());
			TRANSCEIVER_DefaultRx();
		} else {
			int dly = settings.mac.toa_dly.fin_dly_us;
			int lag = MAC_UsFromRx();
			int tx_time = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_TOA_RESP_s));
			TOA_TRACE("TOA FIN sent to %X (%d>%d+%d)", toa.core.addr_tab[0], dly, lag, tx_time);
		}
	} else {
		int ret = TOA_EnableRxBeforeFin(&toa.core, &settings.mac.toa_dly, toa.core.TsPollRx);
		if (ret != 0) {
			int lag = MAC_UsFromRx();
			TOA_TRACE("TOA RESP->RESP rx timeout (%d, %d)", lag, toa.core.resp_ind);
		}
	}
	return 0;
}

static int FC_TOA_FIN_cb(const void* data, const prot_packet_info_t* info) {
	TOA_State(&toa.core, TOA_FIN_REC);
	FC_TOA_FIN_s* packet = (FC_TOA_FIN_s*)data;
	int ts_len = toa.core.anc_in_poll_cnt * sizeof(*packet->TsRespRx);
	TOA_ASSERT(packet->FC == FC_TOA_FIN);
	PROT_CHECK_LEN(FC_TOA_FIN, packet->len, sizeof(FC_TOA_FIN_s) + ts_len);
	TOA_ASSERT(sizeof(*packet->TsRespRx) == sizeof(*toa.core.TsRespRx));
	TOA_ASSERT(sizeof(packet->TsFinTxBuf) == 5);

	if (toa.core.prev_state != TOA_RESP_SENT) {
		TOA_TRACE("TOA FIN rx in bad state: %d", toa.core.prev_state);
		return 0;
	}

	// read timestamps
	int64_t TsFinRx40b = TRANSCEIVER_GetRxTimestamp();
	int64_t TsFinTx40b = TOA_Read40bValue(packet->TsFinTxBuf);
	int TsRespRxSize = sizeof(packet->TsRespRx[0]) * toa.core.anc_in_poll_cnt;
	memcpy(toa.core.TsRespRx, packet->TsRespRx, TsRespRxSize);
	toa.core.TsFinTx = TsFinTx40b;
	toa.core.TsPollTx = packet->TsPollTx;
	toa.core.TsFinRx = TsFinRx40b;

	if (toa.core.resp_ind < TOA_MAX_DEV_IN_POLL) {
		// when it was to you
		int tof_dw = TOA_CalcTofDwTu(&toa.core, toa.core.resp_ind);
		int dist_cm = TOA_TofToCm(tof_dw * DWT_TIME_UNITS);
		TOA_MeasurePushLocal(info->original_src, dist_cm);
	} else {
		TOA_TRACE("Dist err");
	}

	// turn on receiver after FIN callback
	dwt_forcetrxoff();
	TRANSCEIVER_DefaultRx();
	return 0;
}

void FC_TOA_RES_cb(const void* data, const prot_packet_info_t* info) {
	FC_TOA_RES_s* packet = (FC_TOA_RES_s*)data;
	TOA_ASSERT(packet->FC == FC_TOA_RES);
	if (packet->len < sizeof(*packet)) {
		LOG_ERR(ERR_BAD_OPCODE_LEN, "FC_TOA_FIN", packet->len, sizeof(*packet));
		return;
	}
	TOA_MeasurePush(&packet->meas);
}

int TOA_RxCb(const void* data, const prot_packet_info_t* info) {
	int ret;
	switch (*(uint8_t*)data) {
		case FC_TOA_POLL:
			FC_TOA_POLL_cb(data, info);
			ret = 1;
			break;
		case FC_TOA_RESP:
			FC_TOA_RESP_cb(data, info);
			ret = 1;
			break;
		case FC_TOA_FIN:
			FC_TOA_FIN_cb(data, info);
			ret = 1;
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

int TOA_TxCb(int64_t TsDwTx) {
	int ret = 0;
	switch (toa.core.state) {
		case TOA_POLL_WAIT_TO_SEND:
			TOA_State(&toa.core, TOA_POLL_SENT);
			toa.core.TsPollTx = TsDwTx;
			TOA_TRACE("TOA POLL sent");
			ret = 1;
			break;
		case TOA_RESP_WAIT_TO_SEND:
			TOA_State(&toa.core, TOA_RESP_SENT);
			toa.core.TsRespTx = TsDwTx;
			int resp_us = (toa.core.TsRespTx - (uint32_t)toa.core.TsPollRx) / UUS_TO_DWT_TIME;
			TOA_TRACE("TOA RESP sent after %dus from POLL RX", resp_us);
			ret = 1;
			break;
		case TOA_FIN_WAIT_TO_SEND:
			TRANSCEIVER_DefaultRx();
			TOA_State(&toa.core, TOA_FIN_SENT);
			toa.core.TsFinTx = TsDwTx;
			int fin_us = ((TsDwTx - toa.core.TsPollTx) & MASK_40BIT) / UUS_TO_DWT_TIME;
			TOA_TRACE("TOA FIN sent after %dus from POLL TX", fin_us);
			ret = 0;  // to release transceiver
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

int TOA_RxToCb() {
	// 2 - temp - to toa module - error - change to 1
	// 1 - to toa module
	// 0 - not to toa module
	int ret = 0;
	switch (toa.core.state) {
		case TOA_POLL_SENT:
			ret = 2;
			break;
		case TOA_RESP_REC:
			ret = 2;
			break;
			// ponizszy fragment kodu jest potrzebny dla multipolingu
			// ale niestety nie jest dopracowany, w przypadku unipollingu
			// zdaza sie (podczas pomiarow co najmniej 2 urzadzen z jednym achorem)
			// ze urzadzenie odpowiada wiadomoscia FIN w nieodpowiednim czasie,
			// tak ze w rezultacie obliczona odleglosc jest bledna (np ujemna)
			// ze wzgledu na posiadanie niesponych stopek czasowych podczas obliczen
			toa.core.TsRespRx[toa.core.resp_ind++] = 0;
			if (toa.core.resp_ind >= toa.core.anc_in_poll_cnt) {
				TOA_SendFinal();
				ret = 1;  // it was timeout to toa module
			} else {
				ret = 2;  // default rx to catch next resp
			}
			break;
			// if status is TOA_RESP_SENT and timeout arrive, then
			// there is some trouble with final message transmiting
			// so abort ranging
		case TOA_RESP_SENT: {
			int fin_to_dw = TRANSCEIVER_GetTime() - TRANSCEIVER_GetTxTimestamp();
			TOA_TRACE("TOA FIN TO after %d", fin_to_dw / UUS_TO_DWT_TIME);
			ret = 2;
			break;
		}
		default:
			if (toa.core.state != TOA_IDLE) {
				dwt_forcetrxoff();
				TRANSCEIVER_DefaultRx();
				TOA_State(&toa.core, TOA_IDLE);
			}
			break;
	}

	if (ret == 2) {
		// mac_default_rx_full();
		dwt_forcetrxoff();
		TRANSCEIVER_DefaultRx();
		TOA_State(&toa.core, TOA_IDLE);
		ret = 1;
	}
	return ret;
}
