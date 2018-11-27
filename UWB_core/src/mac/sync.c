#include "sync.h"
#include "mac.h"

sync_instance_t sync;
extern mac_instance_t mac;

#define PROT_CHECK_LEN(FC, len, expected)                \
                                                         \
  do {                                                   \
    if ((len) < (expected)) {                            \
      LOG_ERR(ERR_MAC_BAD_OPCODE_LEN, #FC, (len), (expected)); \
      return -1;                                         \
    }                                                    \
                                                         \
  }                                                      \
                                                         \
  while (0)

#define MOVE_ARRAY_ELEM(ARR_EL, N) ARR_EL[N] = ARR_EL[N - 1]

void SYNC_Init() {
	sync.local_obj.addr = settings.mac.addr;

	toa_settings_t* tset = &settings.mac.sync_dly;
	const int rx_to_tx_delay = 30;  // us
	const float spi_speed = 20e6;   // Hz
	const int POLL_PROCESSING_TIME_US = 270;
	const int RESP_PROCESSING_TIME_US = 200;

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
}

int64_t SYNC_GlobTimeNeig(sync_neighbour_t* neig, int64_t dw_ts) {
	int64_t dt = (dw_ts - neig->update_ts) & MASK_40BIT;
	int64_t res = dw_ts + neig->time_offset;
	res += (int64_t)(neig->time_coeffP[0] * dt);
	return res & MASK_40BIT;
}

int64_t SYNC_GlobTime(int64_t dw_ts) {
	return SYNC_GlobTimeNeig(&sync.local_obj, dw_ts);
}

void SYNC_InitNeighbour(sync_neighbour_t* neig, dev_addr_t addr, int tree_level) {
	memset(neig, 0, sizeof(*neig));
	neig->addr = addr;
	neig->tree_level = tree_level;
}

sync_neighbour_t* SYNC_FindOrCreateNeighbour(dev_addr_t addr, int tree_level) {
	sync_neighbour_t* neig;
	// search
	for (int i = 0; i < SYNC_MAC_NEIGHBOURS; ++i) {
		neig = &sync.neighbour[i];
		if (neig->addr == addr) {
			return neig;
		}
	}
	// create
	for (int i = 0; i < SYNC_MAC_NEIGHBOURS; ++i) {
		neig = &sync.neighbour[i];
		if (neig->addr == 0) {
			SYNC_InitNeighbour(neig, addr, tree_level);
			return neig;
		}
	}
	return 0;
}

// convert (0):(64B) to (-20B):(+20B)
int64_t SYNC_TrimDrift(int64_t drift) {
	drift &= MASK_40BIT;
	if (drift > (MASK_40BIT >> 1)) {
		drift -= MASK_40BIT + 1;
	}
	return drift;
}

// sigmoid function
float SYNC_Smooth(float x, float range) {
	return x / sqrtf(range + x * x);
}

// time coefficient P calculation
float SYNC_CalcTimeCoeff(sync_neighbour_t* neig) {
	const float K = 0.5;  // algorithm speed (0..1)
	float* X = neig->time_coeffP_raw;
	float* Y = neig->time_coeffP;
	float out = Y[1] + K * (X[0] - X[1]) + 0.1f * X[0];

	neig->time_drift_sum += X[0];
	out += neig->time_drift_sum * 0.001;

	return SYNC_Smooth(out, 1.0f);
}

// shift data arrays and calculate raw data
void SYNC_UpdateNeighbour(sync_neighbour_t* neig, int64_t ext_time, int64_t loc_time, int tof_dw) {
	const int64_t thres = 123456;
	float neig_dt = (loc_time - neig->update_ts) & MASK_40BIT;

	// filter tof
	if (tof_dw > 0) {
		neig->tof_dw = (neig->tof_dw + tof_dw) * 0.5f;
	}

	// update time_offset
	int64_t delta = neig_dt * neig->time_coeffP[0];
	neig->time_offset += delta;
	neig->time_offset &= MASK_40BIT;
	neig->update_ts = loc_time;

	// calc and print drift
	int64_t drift = ext_time + (int64_t)neig->tof_dw;
	drift -= neig->time_offset;     // apply neighbour offset (after add dT*P)
	drift -= loc_time;              // apply local time flow
	drift = SYNC_TrimDrift(drift);  // convert to -20B:+20B value
	int dt_us = neig_dt / 64e9 * 1e6;
	SYNC_TRACE("SYNC %X %12d %7d %9X %10u %X %d", neig->addr, (int )drift,
	           (int )(1e8 * neig->time_coeffP[0]), (uint32_t )(neig->time_offset), dt_us,
	           (int )(SYNC_GlobTimeNeig(neig, loc_time) - loc_time),
	           (int )(1e8 * neig->time_drift_sum));
	SYNC_TIME_DUMP("SYNC %X%08X %X%08X %d %d", (int )(loc_time >> 32), (uint32_t )loc_time,
	               (int )(ext_time >> 32), (uint32_t )ext_time, (int )neig->tof_dw, dt_us);

	// move data in arrays
	MOVE_ARRAY_ELEM(neig->drift, 2);
	MOVE_ARRAY_ELEM(neig->drift, 1);
	MOVE_ARRAY_ELEM(neig->time_coeffP_raw, 2);
	MOVE_ARRAY_ELEM(neig->time_coeffP_raw, 1);
	MOVE_ARRAY_ELEM(neig->time_coeffP, 1);

	// calculate new data
	if (-thres < drift && drift < thres) {
		neig->sync_ready = -thres < drift && drift < thres;
		neig->drift[0] = drift;
		neig->time_coeffP_raw[0] =
		    neig_dt != 0.0 ? ((float)drift) / ((float)neig_dt) : neig->time_coeffP_raw[1];
		neig->time_coeffP_raw[0] = isnanf(neig->time_coeffP_raw[0]) ? 0 : neig->time_coeffP_raw[0];
		neig->time_coeffP[0] = SYNC_CalcTimeCoeff(neig);
		// neig->timeDriftSum += drift;
	} else {
		neig->time_offset += drift;
		neig->time_drift_sum = 0;
		neig->sync_ready = 0;
		neig->drift[0] = 0;
		neig->time_coeffP[0] = 0.0f;
		neig->time_coeffP_raw[0] = 0.0f;
	}
}

void SYNC_UpdateLocalTimeParams(uint64_t transceiver_raw_time) {
	float sum_P = 0;
	int cnt = 0;
	int64_t offset = 0;

	// iterate for each neighbour with nonzero update_ts
	for (int i = 0; i < SYNC_MAC_NEIGHBOURS; ++i) {
		if (sync.neighbour[i].update_ts && sync.neighbour[i].sync_ready) {
			sum_P += sync.neighbour[i].time_coeffP[0];
			offset += sync.neighbour[i].time_offset;  // 63b / 40b = 23b
			++cnt;
		}
	}

	// when there is some synchronized neighbour, apply their clock change rate
	if (cnt != 0) {
		int64_t dt = sync.local_obj.update_ts - transceiver_raw_time;
		sync.local_obj.update_ts = transceiver_raw_time;
		sync.local_obj.time_offset += dt * sync.local_obj.time_coeffP[0];
		sync.local_obj.time_coeffP[0] = sum_P / cnt;
	}
}

// complex synchronisation processing
void SYNC_Update(sync_neighbour_t* neig, int64_t ext_time, int64_t loc_time, int tof_dw) {
	SYNC_UpdateNeighbour(neig, ext_time, loc_time, tof_dw);
	SYNC_UpdateLocalTimeParams(loc_time);

	// add distance to measure table
	if (settings.mac.raport_anchor_anchor_distance) {
		int distance = TOA_TofToCm(tof_dw * DWT_TIME_UNITS);
		TOA_MeasurePushLocal(neig->addr, distance);
	}
}

int SYNC_SendPoll(dev_addr_t dst, dev_addr_t anchors[], int anc_cnt) {
	SYNC_ASSERT(0 < anc_cnt && anc_cnt < TOA_MAX_DEV_IN_POLL);
	SYNC_ASSERT(dst != ADDR_BROADCAST);
	mac_buf_t* buf = MAC_BufferPrepare(dst, false);
	int anc_addr_len = anc_cnt * sizeof(dev_addr_t);
	if (buf == 0) {
		return -1;
	}

	FC_SYNC_POLL_s packet = { .FC = FC_SYNC_POLL, .len = sizeof(FC_SYNC_POLL_s)
	    + anc_cnt * sizeof(dev_addr_t), .num_poll_anchor = anc_cnt, };
	MAC_Write(buf, &packet, sizeof(FC_SYNC_POLL_s));
	MAC_Write(buf, anchors, sizeof(dev_addr_t) * anc_cnt);

	sync.toa.resp_ind = 0;
	sync.toa.anc_in_poll_cnt = anc_cnt;
	sync.toa.initiator = settings.mac.addr;
	memcpy(&sync.toa.addr_tab[0], anchors, anc_addr_len);

	// send this frame in your slot (through queue) but with ranging flage
	// mac module should set DWT_RESPONSE_EXPECTED flag before transmission
	buf->isRangingFrame = true;
	TOA_State(&sync.toa, TOA_POLL_WAIT_TO_SEND);
	MAC_SetFrameType(buf, FR_CR_MAC);
	MAC_Send(buf, false);
	return 0;
}

int SYNC_SendResp(int64_t PollDwRxTs) {
	toa_settings_t* tset = &settings.mac.sync_dly;
	int resp_dly_us = tset->resp_dly_us[sync.toa.resp_ind];

	sync.toa.TsRespTx = TOA_SetTxTime(PollDwRxTs, resp_dly_us);

	mac_buf_t* buf = MAC_BufferPrepare(sync.toa.initiator, false);
	if (buf == 0) {
		TOA_State(&sync.toa, TOA_IDLE);
		return 0;
	}
	FC_SYNC_RESP_s packet = { .FC = FC_SYNC_RESP, .len = sizeof(FC_SYNC_RESP_s), .TsPollRx =
	    sync.toa.TsPollRx, .TsRespTx = sync.toa.TsRespTx, };
	MAC_Write(buf, &packet, packet.len);

	int tx_to_rx_dly_us = tset->resp_dly_us[sync.toa.anc_in_poll_cnt - 1];
	tx_to_rx_dly_us += tset->fin_dly_us - resp_dly_us;  // == delay to tx fin
	tx_to_rx_dly_us -= tset->rx_after_tx_offset_us;  // substract time for preamble
	SYNC_ASSERT(tx_to_rx_dly_us > 0);
	dwt_setrxaftertxdelay(tx_to_rx_dly_us);
	dwt_setrxtimeout(settings.mac.sync_dly.guard_time_us + tset->rx_after_tx_offset_us);

	TOA_State(&sync.toa, TOA_RESP_WAIT_TO_SEND);
	const int tx_flags = DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED;
	return MAC_SendRanging(buf, tx_flags);
}

int SYNC_SendFinal() {
	toa_settings_t* tset = &settings.mac.sync_dly;
	int fin_dly_us = tset->resp_dly_us[sync.toa.anc_in_poll_cnt - 1];
	fin_dly_us += tset->fin_dly_us;
	int64_t TsFinTx = TOA_SetTxTime(sync.toa.TsPollTx, fin_dly_us);

	mac_buf_t* buf = MAC_BufferPrepare(sync.toa.addr_tab[0], false);
	FC_SYNC_FIN_s packet = { .FC = FC_SYNC_FIN, .len = sizeof(FC_SYNC_FIN_s), .tree_level =
	    sync.tree_level, .slot_num = mac.slot_number, .TsPollTx = sync.toa.TsPollTx, };
	// calc time offset
	int64_t offset = SYNC_GlobTimeNeig(&sync.local_obj, TsFinTx);
	offset = (offset - TsFinTx) & MASK_40BIT;
	TOA_Write40bValue(&packet.TsOffset[0], offset);
	// calc global time in place to keep 40B resolution
	TOA_Write40bValue(&packet.TsFinTxBuf[0], TsFinTx);

	int resp_rx_ts_len = sizeof(*packet.TsRespRx) * sync.toa.anc_in_poll_cnt;
	packet.len += resp_rx_ts_len;
	memcpy(&packet.TsRespRx[0], &sync.toa.TsRespRx[0], resp_rx_ts_len);
	MAC_Write(buf, &packet, packet.len);

	TOA_State(&sync.toa, TOA_FIN_WAIT_TO_SEND);
	const int tx_flags = DWT_START_TX_DELAYED;
	return MAC_SendRanging(buf, tx_flags);
}

int FC_SYNC_POLL_cb(const void* data, const prot_packet_info_t* info) {
	TOA_State(&sync.toa, TOA_POLL_REC);
	FC_SYNC_POLL_s* packet = (FC_SYNC_POLL_s*)data;
	SYNC_ASSERT(packet->FC == FC_SYNC_POLL);
	PROT_CHECK_LEN(FC_SYNC_POLL, packet->len, sizeof(FC_SYNC_POLL_s));
	int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

	sync.toa_ts_poll_rx_raw = rx_ts;
	sync.toa.TsPollRx = rx_ts;
	sync.toa.initiator = info->original_src;
	sync.toa.anc_in_poll_cnt = packet->num_poll_anchor;
	sync.toa.resp_ind = 0;
	const int addr_len = sizeof(dev_addr_t) * packet->num_poll_anchor;
	memcpy(sync.toa.addr_tab, packet->poll_addr, addr_len);

	// chech if it is full sync to you
	sync.toa.resp_ind = TOA_FindAddrIndexInResp(&sync.toa, settings.mac.addr);
	if (sync.toa.resp_ind < sync.toa.anc_in_poll_cnt) {
		int ret = SYNC_SendResp(rx_ts);
		if (ret != 0) {
			LOG_DBG("Sync POLL->RESP tx timeout (%d)", MAC_UsFromRx());
			TRANSCEIVER_DefaultRx();
		} else {
			int dly = settings.mac.sync_dly.resp_dly_us[sync.toa.resp_ind];
			int lag = MAC_UsFromRx();
			int tx_time = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_SYNC_RESP_s));
			SYNC_TRACE_TOA("SYNC RESP sent to %X (%d>%d+%d)", sync.toa.initiator, dly, lag, tx_time);
		}
	} else {
		// indicate that last poll wasn't to you
		sync.toa.resp_ind = TOA_MAX_DEV_IN_POLL;
	}
	return 0;
}

int FC_SYNC_RESP_cb(const void* data, const prot_packet_info_t* info) {
	TOA_State(&sync.toa, TOA_RESP_REC);
	FC_SYNC_RESP_s* packet = (FC_SYNC_RESP_s*)data;
	SYNC_ASSERT(packet->FC == FC_SYNC_RESP);
	PROT_CHECK_LEN(FC_SYNC_RESP, packet->len, sizeof(FC_SYNC_RESP_s));
	int64_t rx_ts = TRANSCEIVER_GetRxTimestamp();

	if (sync.toa.resp_ind < TOA_MAX_DEV_IN_POLL) {
		sync.toa.TsRespRx[sync.toa.resp_ind++] = rx_ts;
	}
	if (sync.toa.resp_ind >= sync.toa.anc_in_poll_cnt) {
		int ret = SYNC_SendFinal();
		if (ret != 0) {
			SYNC_TRACE_TOA("Sync RESP->FIN tx timeout (%d)", MAC_UsFromRx());
		} else {
			int dly = settings.mac.sync_dly.fin_dly_us;
			int lag = MAC_UsFromRx();
			int tx_time = TRANSCEIVER_EstimateTxTimeUs(sizeof(FC_SYNC_RESP_s));
			SYNC_TRACE_TOA("SYNC FIN sent to %X (%d>%d+%d)", sync.toa.addr_tab[0], dly, lag, tx_time);
		}
	} else {
		int ret = TOA_EnableRxBeforeFin(&sync.toa, &settings.mac.sync_dly, sync.toa_ts_poll_rx_raw);
		if (ret != 0) {
			int lag = MAC_UsFromRx();
			SYNC_TRACE_TOA("Sync RESP->RESP rx timeout (%d, %d)", lag, sync.toa.resp_ind);
		}
	}
	return 0;
}

int FC_SYNC_FIN_cb(const void* data, const prot_packet_info_t* info) {
	TOA_State(&sync.toa, TOA_FIN_REC);
	FC_SYNC_FIN_s* packet = (FC_SYNC_FIN_s*)data;
	int ts_len = sync.toa.anc_in_poll_cnt * sizeof(*packet->TsRespRx);
	SYNC_ASSERT(packet->FC == FC_SYNC_FIN);
	PROT_CHECK_LEN(FC_SYNC_FIN, packet->len, sizeof(FC_SYNC_FIN_s) + ts_len);
	SYNC_ASSERT(sizeof(*packet->TsRespRx) == sizeof(*sync.toa.TsRespRx));
	SYNC_ASSERT(sizeof(packet->TsFinTxBuf) == 5);

	sync_neighbour_t* neig = SYNC_FindOrCreateNeighbour(info->original_src, packet->tree_level);
	if (neig == 0) {
		return 0;
	}

	// read timestamps
	int64_t TsFinRx40b = TRANSCEIVER_GetRxTimestamp();
	int64_t TsFinTx40b = TOA_Read40bValue(packet->TsFinTxBuf);
	int64_t TsOffset40b = TOA_Read40bValue(packet->TsOffset);
	int TsRespRxSize = sizeof(packet->TsRespRx[0]) * sync.toa.anc_in_poll_cnt;
	memcpy(sync.toa.TsRespRx, packet->TsRespRx, TsRespRxSize);
	sync.toa.TsFinTx = TsFinTx40b;
	sync.toa.TsPollTx = packet->TsPollTx;
	sync.toa.TsFinRx = TsFinRx40b;

	int64_t ext_time = (TsFinTx40b + TsOffset40b) & MASK_40BIT;
	int64_t loc_time = TsFinRx40b;

	if (sync.toa.resp_ind < TOA_MAX_DEV_IN_POLL) {
		// when it was to you
		int tof_dw = TOA_CalcTofDwTu(&sync.toa, sync.toa.resp_ind);
		SYNC_Update(neig, ext_time, loc_time, tof_dw);
		SYNC_TRACE_TOA("Dist %d", TOA_TofToCm(tof_dw * DWT_TIME_UNITS));
	} else {
		// todo: sync_brief(&sync.toa, neig);
		SYNC_Update(neig, ext_time, loc_time, 0);
		SYNC_TRACE_TOA("Dist err");
	}

	// turn on receiver after FIN callback
	dwt_forcetrxoff();
	TRANSCEIVER_DefaultRx();
	return 0;
}

int SYNC_RxCb(const void* data, const prot_packet_info_t* info) {
	int ret;
	switch (*(uint8_t*)data) {
		case FC_SYNC_POLL:
			FC_SYNC_POLL_cb(data, info);
			ret = 1;
			break;
		case FC_SYNC_RESP:
			FC_SYNC_RESP_cb(data, info);
			ret = 1;
			break;
		case FC_SYNC_FIN:
			FC_SYNC_FIN_cb(data, info);
			ret = 1;
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

int SYNC_TxCb(int64_t TsDwTx) {
	int ret = 0;
	switch (sync.toa.state) {
		case TOA_POLL_WAIT_TO_SEND:
			TOA_State(&sync.toa, TOA_POLL_SENT);
			sync.toa.TsPollTx = TsDwTx;
			SYNC_TRACE_TOA("SYNC POLL sent");
			ret = 1;
			break;
		case TOA_RESP_WAIT_TO_SEND:
			TOA_State(&sync.toa, TOA_RESP_SENT);
			sync.toa.TsRespTx = TsDwTx;
			int resp_us = (sync.toa.TsRespTx - sync.toa.TsPollRx) / UUS_TO_DWT_TIME;
			SYNC_TRACE_TOA("SYNC RESP sent after %dus", resp_us);
			ret = 1;
			break;
		case TOA_FIN_WAIT_TO_SEND:
			TRANSCEIVER_DefaultRx();
			TOA_State(&sync.toa, TOA_FIN_SENT);
			sync.toa.TsFinTx = TsDwTx;
			int fin_us = ((TsDwTx - sync.toa.TsPollTx) & MASK_40BIT) / UUS_TO_DWT_TIME;
			SYNC_TRACE_TOA("SYNC FIN sent after %dus from POLL", fin_us);
			ret = 0;  // to release transceiver
			break;
		default:
			ret = 0;
			break;
	}
	return ret;
}

int SYNC_RxToCb() {
	// 2 - temp - to sync module - error - change to 1
	// 1 - to sync module
	// 0 - not to sync module
	int ret = 0;
	switch (sync.toa.state) {
		case TOA_POLL_SENT:
		case TOA_RESP_REC:
			sync.toa.TsRespRx[sync.toa.resp_ind++] = 0;
			if (sync.toa.resp_ind >= sync.toa.anc_in_poll_cnt) {
				SYNC_SendFinal();
				ret = 1;  // it was timeout to syn module
			} else {
				ret = 2;  // default rx to catch next resp
			}
			break;
			// if status is TOA_RESP_SENT and timeout arrive, then
			// there is some trouble with final message transmiting
			// so abort ranging
		case TOA_RESP_SENT: {
			int fin_to_dw = TRANSCEIVER_GetTime() - TRANSCEIVER_GetTxTimestamp();
			SYNC_TRACE_TOA("SYNC FIN TO after %d", fin_to_dw / UUS_TO_DWT_TIME);
			ret = 2;
			break;
		}
		default:
			if (sync.toa.state != TOA_IDLE) {
				dwt_forcetrxoff();
				TRANSCEIVER_DefaultRx();
				TOA_State(&sync.toa, TOA_IDLE);
			}
			break;
	}

	if (ret == 2) {
		// mac_default_rx_full();
		dwt_forcetrxoff();
		TRANSCEIVER_DefaultRx();
		TOA_State(&sync.toa, TOA_IDLE);
		ret = 1;
	}
	return ret;
}
