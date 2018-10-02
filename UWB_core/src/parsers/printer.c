#include "printer.h"
#include "mac/carry.h"

void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did) {
	const char *str_role;
	switch (data->role) {
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
	LOG_INF(INF_VERSION, "version %X %s %d.%d %d.%d.%X", did, str_role, data->hMajor, data->hMinor,
	        data->fMajor, data->fMinor, data->hash);
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

	LOG_INF(INF_STATUS, "stat did:%x mV:%d Rx:%d Tx:%d Er:%d To:%d Uptime:%dd.%dh.%dm.%ds", did,
	        data->battery_mV,
	        data->rx_cnt, data->tx_cnt, data->err_cnt, data->to_cnt, days, hr, min, sec);
}

void PRINT_TurnOn(const FC_TURN_ON_s *data, dev_addr_t did) {
	LOG_INF(INF_DEVICE_TURN_ON, "Device turn on did:%X fV:%d", did, data->fMinor);
}

void PRINT_TurnOff(const FC_TURN_OFF_s *data, dev_addr_t did) {
	LOG_INF(INF_DEVICE_TURN_OFF, "Device turn off did:%X", did);
}

void PRINT_Beacon(const FC_BEACON_s *data, dev_addr_t did) {
	LOG_INF(INF_BEACON, "Beacon from did:%X", did);
}

void PRINT_DeviceAccepted(const FC_DEV_ACCEPTED_s *data, dev_addr_t did) {
	LOG_INF(INF_DEV_ACCEPTED, "Device accepted, sink:%X parent:%X", did, CARRY_ParentAddres());
}

void PRINT_SettingsSaveResult(const FC_SETTINGS_SAVE_RESULT_s *data, dev_addr_t did) {
	switch (data->result) {
		case 0:
			LOG_INF(INF_SETTINGS_SAVED, "settings saved did:%X", did);
			break;
		case 1:
			LOG_ERR(ERR_FLASH_ERASING, "flash erasing error did:%X", did);
			break;
		case 2:
			LOG_ERR(ERR_FLASH_WRITING, "flash writing error did:%X", did);
			break;
		case 3:
			LOG_INF(INF_SETTINGS_NO_CHANGES, "no changes to be saved did:%X", did);
			break;
		case 4:
			LOG_WRN(WRN_FIRWARE_NOT_ACCEPTED_YET, "new firmware not accepted yet! did:%X", did);
			break;
		default:
			LOG_ERR(ERR_FLASH_OTHER, "SETTINGS_Save bad implementation did:%X", did);
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
	LOG_INF(INF_RF_SETTINGS,
	        "rfset ch:%d-%d/%d br:%d plen:%d prf:%d pac:%d code:%d nsSfd:%d sfdTo:%d",
	        data->chan, _f[data->chan], _bw[data->chan], br, plen, prf, pac, data->code, data->ns_sfd,
	        data->sfd_to);
}

void PRINT_BleSet(const FC_BLE_SET_s *data, dev_addr_t did) {
	LOG_INF(INF_BLE_SETTINGS, "ble txpower:%d (-40/-20/-16/-12/-8/-4/0/3/4) enable:%d (0/1) did:%X",
	        data->tx_power, data->is_enabled, did);
}

void PRINT_ImuSet(const FC_IMU_SET_s* data, dev_addr_t did) {
	LOG_INF(INF_IMU_SETTINGS, "imu delay:%d enable:%d (0/1) did:%X", data->delay,
          data->is_enabled, did);
}

void PRINT_Measure(const measure_t *data)
{
	LOG_INF(INF_MEASURE_DATA, "a %X>%X %d %d %d %d", data->did1, data->did2, data->dist_cm,
          data->rssi_cdbm, data->fpp_cdbm, data->snr_cdbm);
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
	LOG_INF(INF_MEASURE_INFO, "measure %X with [%s]", data->tagDid, buf);
}

void PRINT_RangingTime() {
	int period = settings.ranging.rangingPeriodMs;
	int delay = settings.ranging.rangingDelayMs;
	LOG_INF(INF_RANGING_TIME, "rangingtime T:%d t:%d (N:%d)", period, delay, period / delay);
}

void PRINT_ToaSettings(const char* prefix, const toa_settings_t *data, dev_addr_t did) {
	const char frm[] = "%s gt:%d fin:%d resp1:%d resp2:%d";
	LOG_INF(INF_TOA_SETTINGS, frm, prefix, data->guard_time_us, data->fin_dly_us,
	        data->resp_dly_us[0], data->resp_dly_us[1]);
}

void PRINT_Parent(dev_addr_t parent, dev_addr_t child, int level) {
	LOG_INF(INF_PARENT_DESCRIPTION, "parent of %X is %X (%d)", child, parent, level);
}
