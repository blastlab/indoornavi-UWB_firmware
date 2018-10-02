#include "txt_parser.h"

/**
 * @brief handle message locally or send it to other device
 * depending on info->direct_src address.
 *
 * @param buf buffer to handle, point at FC field
 * @param info packet extra informations
 */
static void _TXT_Finalize(const void* buf, const prot_packet_info_t* info) {
	prot_packet_info_t new_info;
	if (info->original_src == ADDR_BROADCAST) {
		memset(&new_info, 0, sizeof(new_info));
		new_info.original_src = settings.mac.addr;
		BIN_ParseSingle(buf, info);
	} else {
		FC_CARRY_s* carry;
		mac_buf_t* mbuf = CARRY_PrepareBufTo(info->original_src, &carry);
		if (mbuf != 0) {
			uint8_t* ibuf = (uint8_t*)buf;
			// length is always second byte of frame
			CARRY_Write(carry, mbuf, buf, ibuf[1]);
			CARRY_Send(mbuf, true);
		}
	}
}

/**
 * @brief fully handle ASK type messages without extra parameters
 *
 * @param info extra packet informations
 * @param FC detailed ask function code descriptor
 */
static void _TXT_Ask(const prot_packet_info_t* info, FC_t FC) {
	uint8_t buf[2] = { FC, 2 };
	_TXT_Finalize(buf, info);
}

// === callbacks ===

static void TXT_StatCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	_TXT_Ask(info, FC_STAT_ASK);
}

static void TXT_VersionCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	_TXT_Ask(info, FC_VERSION_ASK);
}

static void TXT_HangCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	LOG_DBG("HANG");
	while (1) {
	}
}

static bool _RFSet_ValidateAndTranslateA(int* ch, int* br, int* plen, int* prf) {
	bool some_change = false;

	if (*ch >= 0) {
		if (!(0 < *ch && *ch < 8 && *ch != 6)) {
			LOG_ERR(ERR_RF_BAD_CHANNEL);
			*ch = -1;
		}
		some_change = true;
	}
	if (*br >= 0) {
		if (*br == 110) {
			*br = DWT_BR_110K;
		} else if (*br == 850) {
			*br = DWT_BR_850K;
		} else if (*br == 6800) {
			*br = DWT_BR_6M8;
		} else {
			LOG_ERR(ERR_RF_BAD_BAUDRATE);
			*br = -1;
		}
		some_change = true;
	}
	if (*plen >= 0) {
		if (*plen == 64)
			*plen = DWT_PLEN_64;  // standard
		else if (*plen == 128)
			*plen = DWT_PLEN_128;
		else if (*plen == 256)
			*plen = DWT_PLEN_256;
		else if (*plen == 512)
			*plen = DWT_PLEN_512;
		else if (*plen == 1024)
			*plen = DWT_PLEN_1024;  // standard
		else if (*plen == 1536)
			*plen = DWT_PLEN_1536;
		else if (*plen == 2048)
			*plen = DWT_PLEN_2048;
		else if (*plen == 4096)
			*plen = DWT_PLEN_4096;  // standard
		else {
			LOG_ERR(ERR_RF_BAD_PREAMBLE_LEN);
			*plen = -1;
		}
		some_change = true;
	}
	if (*prf >= 0) {
		if (*prf == 64)
			*prf = DWT_PRF_64M;
		else if (*prf == 16)
			*prf = DWT_PRF_16M;
		else {
			LOG_ERR(ERR_RF_BAD_PRF);
			*prf = -1;
		}
		some_change = true;
	}
	return some_change;
}

static bool _RFSet_ValidateAndTranslateB(int* pac, int* code, int* nssfd, int* power) {
	bool some_change = false;
	if (*pac >= 0) {
		if (*pac == 8)
			*pac = DWT_PAC8;
		else if (*pac == 16)
			*pac = DWT_PAC16;
		else if (*pac == 32)
			*pac = DWT_PAC32;
		else if (*pac == 64)
			*pac = DWT_PAC64;
		else {
			LOG_ERR(ERR_RF_BAD_PAC);
			*pac = -1;
		}
		some_change = true;
	}
	if (*code >= 0 && !(0 < *code && *code < 25)) {
		LOG_ERR(ERR_RF_BAD_CODE);
		*code = -1;
		some_change = true;
	}
	if (*nssfd >= 0 && !(*nssfd == 0 || *nssfd == 1)) {
		LOG_ERR(ERR_RF_BAD_NSSFD);
		some_change = true;
	}
	if (*power != -1) {
		some_change = true;
	}
	return some_change;
}

// todo: przeniesc obsluge zadania do parse bin poprzez _TXT_ASK
static void TXT_RFSetCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	// read data
	int ch = TXT_GetParam(buf, "ch:", 10);
	int br = TXT_GetParam(buf, "br:", 10);
	int plen = TXT_GetParam(buf, "plen:", 10);
	int prf = TXT_GetParam(buf, "prf:", 10);
	int pac = TXT_GetParam(buf, "pac:", 10);
	int code = TXT_GetParam(buf, "code:", 10);
	int sfdto = TXT_GetParam(buf, "sfdto:", 10);
	int pgdly = TXT_GetParam(buf, "pgdly:", 10);
	int nssfd = TXT_GetParam(buf, "nssfd:", 10);
	int power = TXT_GetParam(buf, "power:", 16);

	// validate values
	bool changes = false;
	changes |= _RFSet_ValidateAndTranslateA(&ch, &br, &plen, &prf);
	changes |= _RFSet_ValidateAndTranslateB(&pac, &code, &nssfd, &power);

	// fill struct with a ridden or ignored data
	FC_RF_SET_s packet;
	packet.FC = changes ? FC_RFSET_SET : FC_RFSET_ASK;
	packet.len = changes ? sizeof(packet) : 2;
	packet.chan = ch >= 0 ? ch : 255;
	packet.br = br >= 0 ? br : 255;
	packet.plen = plen >= 0 ? plen : 255;
	packet.prf = prf >= 0 ? prf : 255;
	packet.pac = pac >= 0 ? pac : 255;
	packet.code = code >= 0 ? code : 255;
	packet.sfd_to = sfdto >= 0 ? sfdto : 0;
	packet.pg_dly = pgdly >= 0 ? pgdly : 0;
	packet.ns_sfd = nssfd >= 0 ? nssfd : 255;
	packet.power = power != -1 ? power : 0;

	_TXT_Finalize(&packet, info);
}

static void TXT_TestCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	LOG_TEST(TEST_PASS);
}

static void TXT_SaveCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	_TXT_Ask(info, FC_SETTINGS_SAVE);
}

static void TXT_ClearCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	bool clear_both = TXT_CheckFlag(buf, "-pm") | TXT_CheckFlag(buf, "-mp");
	bool clear_parent = TXT_CheckFlag(buf, "-p") | clear_both;
	bool clear_measures = TXT_CheckFlag(buf, "-m") | clear_both;
	bool cleared = false;

	if (clear_measures) {
		RANGING_MeasureDeleteAll();
		cleared = true;
	}
	if (clear_parent) {
		CARRY_ParentDeleteAll();
		cleared = true;
	}
	if (cleared) {
		if (clear_both) {
			LOG_INF(INF_CLEARED_PARENTS_AND_MEASURES);
		} else if (clear_parent) {
			LOG_INF(INF_CLEARED_PARENTS);
		} else if (clear_measures) {
			LOG_INF(INF_CLEARED_MEASURES);
		}
	} else {
		LOG_INF(INF_CLEAR_HELP);
	}

}

static void TXT_ResetCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	_TXT_Ask(info, FC_RESET);
}

static void TXT_BinCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	mac_buf_t* data = MAC_Buffer();
	if (data != 0) {
		data->isServerFrame = true;
		// copy base64 string to continuum memory space
		data->dPtr = data->buf;
		const char* cmd = TXT_PointParamNumber(buf, buf->cmd, 1);
		while (cmd[0] != 0) {
			if (data->dPtr > data->buf + MAC_BUF_LEN) {
				LOG_ERR(ERR_BASE64_TOO_LONG_INPUT);
				return;
			}
			data->dPtr[0] = cmd[0];
			++data->dPtr;
			INCREMENT_CYCLE(cmd, buf->start, buf->end);
		}
		data->dPtr = data->buf;
		// decode oryginal content and parse
		int size = BASE64_Decode(data->buf, data->buf, MAC_BUF_LEN);
		BIN_Parse(data->dPtr, info, size);
		data->isServerFrame = false;
		MAC_Free(data);
	}
}

static void TXT_SetAnchorsCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	int res, i = 1;
	RANGING_TempAnchorsReset();
	res = TXT_GetParamNum(buf, i, 16);
	while (res > 0) {
		if (!RANGING_TempAnchorsAdd(res) || (res & ADDR_ANCHOR_FLAG) == 0) {
			LOG_ERR(ERR_SETANCHORS_FAILED, res);
			RANGING_TempAnchorsReset();
			return;
		}
		++i;
		res = TXT_GetParamNum(buf, i, 16);
	}
	LOG_INF(INF_SETANCHORS_SET, i - 1);
}

static void TXT_SetTagsCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	int res, i = 1;
	if (RANGING_TempAnchorsCounter() == 0) {
		LOG_ERR(ERR_SETTAGS_NEED_SETANCHORS);
		return;
	}
	res = TXT_GetParamNum(buf, i, 16);
	while (res > 0) {
		if (!RANGING_AddTagWithTempAnchors(res, 1)) {
			LOG_ERR(ERR_SETTAGS_FAILED, res);
			return;
		}
		++i;
		res = TXT_GetParamNum(buf, i, 16);
	}
	int anchors = RANGING_TempAnchorsCounter();
	LOG_INF(INF_SETTAGS_SET, i - 1, anchors);
}

static void TXT_MeasureCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	static int readIt = 0;
	int i = 2;
	int tagDid = TXT_GetParamNum(buf, 1, 16);
	int ancDid = TXT_GetParamNum(buf, i, 16);

	if (tagDid < 0 || tagDid > ADDR_BROADCAST) {  // no parameters
		LOG_INF(INF_MEASURE_CMD_CNT, RANGING_MeasureCounter());
		return;
	} else if (tagDid == ADDR_BROADCAST) {  // one parameter - ADDR_BROADCAST
		readIt = readIt >= settings.ranging.measureCnt ? 0 : readIt;
		PRINT_MeasureInitInfo(&settings.ranging.measure[readIt]);
		INCREMENT_MOD(readIt, settings.ranging.measureCnt);
		return;
	}
	RANGING_TempAnchorsReset();
	while (ancDid > 0) {
		if (!RANGING_TempAnchorsAdd(ancDid)) {
			LOG_ERR(ERR_MEASURE_FAILED_DID, ancDid);
			RANGING_TempAnchorsReset();
			return;
		}
		++i;
		ancDid = TXT_GetParamNum(buf, i, 16);
	}
	if (!RANGING_AddTagWithTempAnchors(tagDid, RANGING_TempAnchorsCounter())) {
		LOG_ERR(ERR_MEASURE_FAILED_ANC_CNT, RANGING_TempAnchorsCounter());
		RANGING_TempAnchorsReset();
		return;
	}
	LOG_INF(INF_MEASURE_CMD_SET, tagDid, RANGING_TempAnchorsCounter());
	RANGING_TempAnchorsReset();
}

static void TXT_DeleteTagsCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	int res, i = 1, deleted = 0;
	res = TXT_GetParamNum(buf, i, 16);
	while (res > 0) {
		if (RANGING_MeasureDeleteTag(res)) {
			++deleted;
		}
		++i;
		res = TXT_GetParamNum(buf, i, 16);
	}
	LOG_INF(INF_DELETETAGS, deleted);
}

static void TXT_RangingTimeCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	int period = TXT_GetParam(buf, "T:", 10);
	int delay = TXT_GetParam(buf, "t:", 10);
	int cnt = TXT_GetParam(buf, "N:", 10);

	if (period < 0 && delay < 0 && cnt < 0) {
		PRINT_RangingTime();
		return;
	}

	int meas_count = RANGING_MeasureCounter();
	delay = delay > 0 ? delay : settings.ranging.rangingDelayMs;
	period = cnt > 0 ? cnt * delay : period;
	period = period > 0 ? period : settings.ranging.rangingPeriodMs;
	cnt = cnt > 0 ? cnt : period / delay;

	if (meas_count > cnt) {
		cnt = meas_count;
		period = cnt * delay;
		LOG_ERR(WRN_RANGING_TOO_SMALL_PERIOD, cnt, period);
	}

	settings.ranging.rangingDelayMs = delay;
	settings.ranging.rangingPeriodMs = period;
	PRINT_RangingTime();
}

static void TXT_ToaTimeCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	toa_settings_t* set = &settings.mac.toa_dly;
	const char prefix[] = "toatime";
	char resp_cmd[] = "resp_:";
	bool respPresent = false;
	int gt = TXT_GetParam(buf, "gt:", 10);
	int fin = TXT_GetParam(buf, "fin:", 10);
	int resp[TOA_MAX_DEV_IN_POLL];

	for (int i = 0; i < TOA_MAX_DEV_IN_POLL; ++i) {
		resp_cmd[4] = i + '1';
		resp[i] = TXT_GetParam(buf, resp_cmd, 10);
		respPresent = resp[i] > 0 ? true : respPresent;
		resp[i] = resp[i] > 0 ? resp[i] : set->resp_dly_us[i];
	}

	if (gt < 0 && fin < 0 && respPresent == false) {
		PRINT_ToaSettings(prefix, set, settings.mac.addr);
		return;
	}

	gt = gt > 0 ? gt : set->guard_time_us;
	fin = fin > 0 ? fin : set->fin_dly_us;

	set->guard_time_us = gt;
	set->fin_dly_us = fin;
	for (int i = 0; i < TOA_MAX_DEV_IN_POLL; ++i) {
		set->resp_dly_us[i] = resp[i];
	}
	PRINT_ToaSettings(prefix, set, settings.mac.addr);
}

static void TXT_AutoSetupCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	int en = TXT_GetParam(buf, "en:", 10);

	if (en < 0) {
	} else {
		// todo: przerobic na pakiet binarny i binparse
		settings.mac.raport_anchor_anchor_distance = en > 0;
	}
}

static void TXT_ParentCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	int parent = TXT_GetParamNum(buf, 1, 16);
	int child = TXT_GetParamNum(buf, 2, 16);
	int fail_cnt = 0;
	int i = 2;
	int level = 0;
	static int readIt = 0;

	if (parent <= 0 && child <= 0) {  // no parametrs
		LOG_INF(INF_PARENT_CNT, settings.carry.targetCounter);
		return;
	} else if (child <= 0) {  // one parametr
		child = parent;
		if (child == ADDR_BROADCAST) {
			readIt = readIt >= settings.carry.targetCounter ? 0 : readIt;
			child = settings.carry.target[readIt].addr;
			INCREMENT_MOD(readIt, settings.carry.targetCounter);
		}
		parent = CARRY_ParentGet(child);
		dev_addr_t temp_parent = parent;
		while (temp_parent != ADDR_BROADCAST) {
			temp_parent = CARRY_ParentGet(temp_parent);
			++level;
		}
		PRINT_Parent(parent, child, level);
	} else if ((parent & ADDR_ANCHOR_FLAG) == 0 || parent > 0xFFFF) {
		LOG_ERR(ERR_PARENT_NEED_ANCHOR, parent);
		return;
	} else {
		while (child > 0) {
			if (child == settings.mac.addr) {
				LOG_ERR(ERR_PARENT_FOR_SINK);
			} else {
				if (CARRY_ParentSet(child, parent) == 0) {
					++fail_cnt;
				}
			}
			++i;
			child = TXT_GetParamNum(buf, i, 16);
		}
		LOG_INF(INF_PARENT_SET, parent, i - 2 - fail_cnt, fail_cnt);
	}
}

static void TXT_BleCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
	BLE_CODE(
			int power = TXT_GetParam(buf, "txpower:", 10);
			int enable = TXT_GetParam(buf, "enable:", 10);

			FC_BLE_SET_s packet; uint8_t changes = 0; switch (power) {
				case -40:
				case -20:
				case -16:
				case -12:
				case -8:
				case -4:
				case 0:
				case 3:
				case 4:
				packet.tx_power = power;
				changes++;
				break;
				default:
				packet.tx_power = -1;
			}switch (enable) {
				case 0:
				case 1:
				packet.is_enabled = enable;
				changes++;
				break;
				default:
				packet.is_enabled = -1;
			}packet.FC = changes ? FC_BLE_SET : FC_BLE_ASK;
			packet.len = changes ? sizeof(packet) : 2;

			_TXT_Finalize(&packet, info);
			return;
	)
	LOG_ERR(ERR_BLE_INACTIVE);
}

static void TXT_ImuCb(const txt_buf_t* buf, const prot_packet_info_t* info) {
  int delay = TXT_GetParam(buf, "delay:", 10);
  int enable = TXT_GetParam(buf, "enable:", 10);

  FC_IMU_SET_s packet;
  uint8_t changes = 0;
  if (delay >= 10) {
    packet.delay = delay;
    changes++;
  } else {
    packet.delay = -1;
  }
  switch (enable) {
    case 0:
    case 1:
      packet.is_enabled = enable;
      changes++;
      break;
    default:
      packet.is_enabled = -1;
  }
  packet.FC = changes ? FC_IMU_SET : FC_IMU_ASK;
  packet.len = changes ? sizeof(packet) : 2;

  _TXT_Finalize(&packet, info);
}

static void TXT_Role(const txt_buf_t* buf, const prot_packet_info_t* info) {
	const char* role = TXT_PointParamNumber(buf, buf->cmd, 1);
	switch (tolower(role[0])) {
		case 's':
			settings.mac.role = RTLS_SINK;
			break;
		case 'a':
			settings.mac.role = RTLS_ANCHOR;
			break;
		case 't':
			settings.mac.role = RTLS_TAG;
			break;
		case 'l':
			settings.mac.role = RTLS_LISTENER;
			break;
		default:
			break;
	}
	// print version
	_TXT_Ask(info, FC_VERSION_ASK);
}

static void TXT_Route(const txt_buf_t* buf, const prot_packet_info_t* info) {
	int en = TXT_GetParam(buf, "auto:", 10);

	if (0 <= en && en <= 1) {
		settings.carry.autoRoute = en;
	}

	LOG_INF(INF_ROUTE, settings.carry.autoRoute);
}

const txt_cb_t txt_cb_tab[] = {
    { "stat", TXT_StatCb },
    { "version", TXT_VersionCb },
    { "_hang", TXT_HangCb },
    { "rfset", TXT_RFSetCb },
    { "test", TXT_TestCb },
    { "save", TXT_SaveCb },
    { "clear", TXT_ClearCb },
    { "reset", TXT_ResetCb },
    { "bin", TXT_BinCb },
    { "setanchors", TXT_SetAnchorsCb },
    { "settags", TXT_SetTagsCb },
    { "measure", TXT_MeasureCb },
    { "deletetags", TXT_DeleteTagsCb },
    { "rangingtime", TXT_RangingTimeCb },
    { "toatime", TXT_ToaTimeCb },
    { "_autosetup", TXT_AutoSetupCb },
    { "parent", TXT_ParentCb },
    { "ble", TXT_BleCb },
    { "imu", TXT_ImuCb},
    { "_role", TXT_Role },
    { "route", TXT_Route }, };

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);
