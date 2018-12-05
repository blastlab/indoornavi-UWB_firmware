#include "printer.h"
#include "mac/carry.h"
#include "mac/toa_routine.h"

const char* RoleToString(rtls_role role) {
	const char *str_role;
	switch (role) {
		case RTLS_SINK:
			str_role = "SINK";
			break;
		case RTLS_ANCHOR:
			str_role = "ANCHOR";
			break;
		case RTLS_TAG:
			str_role = "TAG";
			break;
		case RTLS_LISTENER:
			str_role = "LISTENER";
			break;
		case RTLS_DEFAULT:
			str_role = "DEFAULT";
			break;
		default:
			str_role = "OTHER";
	}
	return str_role;
}

void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did) {
	const char *str_role = RoleToString(data->role);
	LOG_INF(INF_VERSION, did, (uint32_t)(data->serial >> 32), (uint32_t)data->serial, str_role,
	        data->hMajor, data->hMinor, data->fMajor, data->fMinor, (uint32_t)(data->hash >> 32),
		(uint32_t)(data->hash));
}

void PRINT_Stat(const FC_STAT_s *data, dev_addr_t did) {
	int offset = 0;
	int days = data->uptime_ms / (1000 * 60 * 60 * 24);
	offset = days * 24;
	int hr = data->uptime_ms / (1000 * 60 * 60) - offset;
	offset = (offset + hr) * 60;
	int min = data->uptime_ms / (1000 * 60) - offset;
	offset = (offset + min) * 60;
	int sec = data->uptime_ms / (1000) - offset;

	LOG_INF(INF_STATUS, did, data->battery_mV, data->rx_cnt, data->tx_cnt, data->err_cnt,
	        data->to_cnt, days, hr, min, sec);
}

void PRINT_TurnOn(const FC_TURN_ON_s *data, dev_addr_t did) {
	LOG_INF(INF_DEVICE_TURN_ON, did, data->fMinor);
}

void PRINT_TurnOff(const FC_TURN_OFF_s *data, dev_addr_t did) {
	LOG_INF(INF_DEVICE_TURN_OFF, did);
}

void PRINT_Beacon(const FC_BEACON_s *data, dev_addr_t did, dev_addr_t hops[]) {
	mac_buf_t* mbuf = MAC_Buffer();
	char* buf;
	if (mbuf == 0) {
		buf = "";
	} else {
		buf = (char*)mbuf->buf;
		for (int i = 0; i < (data->hop_cnt_batt >> 4); ++i) {
			snprintf(buf + strlen(buf), MAC_BUF_LEN, "%X>", hops[i]);
		}
		snprintf(buf, MAC_BUF_LEN, "%X", did);
	}
	int mV = data->hop_cnt_batt & 0x0F;
	mV = (mV << 8) + data->voltage; 
	mV = mV < 100 ? 0 : mV + 2000; // 2000 is battery offset level to compress value into 12B field
	LOG_INF(INF_BEACON, did, mV, buf, data->serial_hi, data->serial_lo);
	MAC_Free(mbuf);
}

void PRINT_DeviceAccepted(const FC_DEV_ACCEPTED_s *data, dev_addr_t did) {
	LOG_INF(INF_DEV_ACCEPTED, did, CARRY_ParentAddres());
}

void PRINT_SettingsSaveResult(const FC_SETTINGS_SAVE_RESULT_s *data, dev_addr_t did) {
	switch (data->result) {
		case 0:
			LOG_INF(INF_SETTINGS_SAVED, did);
			break;
		case 1:
			LOG_ERR(ERR_FLASH_ERASING, did);
			break;
		case 2:
			LOG_ERR(ERR_FLASH_WRITING, did);
			break;
		case 3:
			LOG_INF(INF_SETTINGS_NO_CHANGES, did);
			break;
		case 4:
			LOG_WRN(WRN_FIRWARE_NOT_ACCEPTED_YET, did);
			break;
		default:
			LOG_ERR(ERR_FLASH_OTHER, did);
			break;
	}
}

void PRINT_RFSet(const FC_RF_SET_s *data, dev_addr_t did) {
	const int _f[8] = { 0, 3494, 3994, 4493, 3994, 6490, 0, 6490 };
	const int _bw[8] = { 0, 499, 499, 499, 1331, 499, 0, 1082 };
	const int _br[3] = { 110, 850, 6800 };
	const int _pac[4] = { 8, 16, 32, 64 };
	int br = _br[data->br];
	int prf = (data->prf == DWT_PRF_16M) ? 16 : 64;
	int pac = _pac[data->pac];
	int plen;
	switch (data->plen) {
		case DWT_PLEN_64:
			plen = 64;
			break;
		case DWT_PLEN_128:
			plen = 128;
			break;
		case DWT_PLEN_256:
			plen = 256;
			break;
		case DWT_PLEN_512:
			plen = 512;
			break;
		case DWT_PLEN_1024:
			plen = 1024;
			break;
		case DWT_PLEN_1536:
			plen = 1536;
			break;
		case DWT_PLEN_2048:
			plen = 2048;
			break;
		case DWT_PLEN_4096:
			plen = 4096;
			break;
		default:
			plen = 0;
			break;
	}
	LOG_INF(INF_RF_SETTINGS, data->chan, _f[data->chan], _bw[data->chan], br, plen, prf, pac,
	        data->code, data->ns_sfd, data->sfd_to, data->smart_tx);
}

static inline int TxSetCoarseGain(int byte) {
	return 18 - 3 * ((byte >> 5) & 0x7);
}

static inline int TxSetFineGainFull(int byte) {
	return (byte & 0x1F) / 2;
}

static inline int TxSetFineGainHalf(int byte) {
	return (byte & 0x1F) % 2;
}

void PRINT_RFTxSet(FC_RF_TX_SET_s* packet, dev_addr_t did) {
	int P4 = (packet->power >> 0) & 0xFF;
	int P3 = (packet->power >> 8) & 0xFF;
	int P2 = (packet->power >> 16) & 0xFF;
	int P1 = (packet->power >> 24) & 0xFF;
	LOG_INF(INF_RF_TX_SETTINGS, did, packet->pg_dly,
		TxSetCoarseGain(P1), TxSetFineGainFull(P1), TxSetFineGainHalf(P1),
		TxSetCoarseGain(P2), TxSetFineGainFull(P2), TxSetFineGainHalf(P2),
		TxSetCoarseGain(P3), TxSetFineGainFull(P3), TxSetFineGainHalf(P3),
		TxSetCoarseGain(P4), TxSetFineGainFull(P4), TxSetFineGainHalf(P4));
}

void PRINT_BleSet(const FC_BLE_SET_s *data, dev_addr_t did) {
	LOG_INF(INF_BLE_SETTINGS, data->tx_power, data->is_enabled, did);
}

void PRINT_ImuSet(const FC_IMU_SET_s* data, dev_addr_t did) {
	LOG_INF(INF_IMU_SETTINGS, data->delay, data->is_enabled, did);
}

void PRINT_Measure(const measure_t *data)
{
#if LOG_USB_EN
	LOG_INF(INF_MEASURE_DATA, data->did1, data->did2, data->dist_cm, data->rssi_cdbm, data->fpp_cdbm,
	        data->snr_cdbm);
#endif
#if LOG_SPI_EN
	FC_TOA_RES_s packet = {
			.FC = FC_TOA_RES,
			.len = sizeof(FC_TOA_RES_s)
	};
	memcpy(&packet.meas, data, sizeof(measure_t));
	LOG_Bin((uint8_t *)&packet, sizeof(measure_t));
#endif
}

void PRINT_MeasureInitInfo(const measure_init_info_t *data) {
	char buf[(1 + sizeof(dev_addr_t)) * TOA_MAX_DEV_IN_POLL + 1];
	buf[0] = 0;
	for (int i = 0; i < data->numberOfAnchors; ++i) {
		int len = strlen(buf);
		snprintf(buf + len, sizeof(buf) - len, "%X,", data->ancDid[i]);
	}
	// delete last ','
	if (buf[0] != 0) {
		buf[strlen(buf) - 1] = 0;
	}
	LOG_INF(INF_MEASURE_INFO, data->tagDid, buf);
}

void PRINT_RangingTime() {
	int period = settings.ranging.rangingPeriodMs;
	int delay = settings.ranging.rangingDelayMs;
	LOG_INF(INF_RANGING_TIME, period, delay, period / delay);
}

void PRINT_ToaSettings(const char* prefix, const toa_settings_t *data, dev_addr_t did) {
	LOG_INF(INF_TOA_SETTINGS, prefix, data->guard_time_us, data->fin_dly_us,
	        data->resp_dly_us[0], data->resp_dly_us[1]);
}

void PRINT_Parent(dev_addr_t parent, dev_addr_t child, int level) {
	LOG_INF(INF_PARENT_DESCRIPTION, child, parent, level);
}

void PRINT_MacSet(const FC_MAC_SET_s *data, dev_addr_t did) {
	const char* str_role = RoleToString(data->role);
	LOG_INF(INF_MAC, did, data->pan, data->beacon_period_ms, data->slot_period_us, data->slot_time_us,
	        data->guard_time_us, data->raport_anchor_to_anchor_distances, str_role);
}
