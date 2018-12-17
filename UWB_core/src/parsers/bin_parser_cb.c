#include "../mac/carry.h"
#include "../mac/toa_routine.h"
#include "../prot/FU.h"
#include "bin_parser.h"
#include "bin_struct.h"
#include "printer.h"

static void SendDevAccepted(dev_addr_t target, dev_addr_t parent) {
  mac_buf_t* buf;
  FC_CARRY_s* carry;
  FC_DEV_ACCEPTED_s acc;
  acc.FC = FC_DEV_ACCEPTED;
  acc.len = sizeof(acc);
  acc.newParent = parent;
	acc.rangingPeriod = settings.ranging.rangingPeriodMs / 100;
  buf = CARRY_PrepareBufTo(target, &carry);
  if (buf != 0) {
		carry->flags |= CARRY_FLAG_REFRESH_PARENT;
    CARRY_Write(carry, buf, &acc, acc.len);
    CARRY_Send(buf, true);
  }
}

static void TransferBeacon(FC_BEACON_s* packet, dev_addr_t hop_cnt_tab[]) {
	mac_buf_t* buf;
	FC_CARRY_s* carry;
	if ((packet->hop_cnt_batt & 0xF0) == 0xF0) {
		LOG_ERR(ERR_BEACON_TOO_MANY_HOPS, packet->hop_cnt_batt >> 4);
		return;
	}
	buf = CARRY_PrepareBufTo(CARRY_ParentAddres(), &carry);
	if (buf != 0) {
		packet->len += sizeof(dev_addr_t);
		packet->hop_cnt_batt += 1 << 4; // (upper nibble)
		CARRY_Write(carry, buf, packet, sizeof(*packet));
		CARRY_Write(carry, buf, &settings.mac.addr, sizeof(dev_addr_t));
		CARRY_Write(carry, buf, hop_cnt_tab, sizeof(dev_addr_t) * ((packet->hop_cnt_batt >> 4) - 1));
		CARRY_Send(buf, false);
	}
}

void BIN_SEND_RESP(FC_t FC, const void* data, uint8_t len, const prot_packet_info_t* info) {
	uint8_t* header = (uint8_t*)data;
	header[0] = (uint8_t)FC;
	header[1] = len;
	FC_CARRY_s* carry;
	mac_buf_t* buf = CARRY_PrepareBufTo(info->original_src, &carry);
	if (buf != 0) {
		CARRY_Write(carry, buf, data, len);
		CARRY_Send(buf, false);
	}
}

void FC_TURN_ON_cb(const void* data, const prot_packet_info_t* info) {
  BIN_ASSERT(*(uint8_t*)data == FC_TURN_ON);
  FC_TURN_ON_s packet;
  memcpy(&packet, data, sizeof(packet));
	PRINT_TurnOn(data, packet.src_did);
  if (settings.mac.role == RTLS_SINK) {
	  if(packet.src_did == info->original_src) {
		  SendDevAccepted(packet.src_did, settings.mac.addr);
	  } else {
		  SendDevAccepted(packet.src_did, info->original_src);
	  }
  } else if (settings.mac.role == RTLS_ANCHOR && packet.src_did != CARRY_ParentAddres()) {
    mac_buf_t* buf;
    FC_CARRY_s* carry;
    buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &carry);
    if (buf != 0) {
      CARRY_Write(carry, buf, &packet, sizeof(packet));
      CARRY_Send(buf, true);
    }
  }
}

void FC_TURN_OFF_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_TURN_OFF);
	PRINT_TurnOff(data, info->original_src);
}

void FC_BEACON_cb(const void* data, const prot_packet_info_t* info) {
  FC_BEACON_s packet;
	FC_BEACON_s* rec_packet = (FC_BEACON_s*)data;
  BIN_ASSERT(*(uint8_t*)data == FC_BEACON);
	int hop_cnt = rec_packet->hop_cnt_batt >> 4;
	int expected_len = sizeof(packet) + sizeof(dev_addr_t) * hop_cnt;
	if (rec_packet->len != expected_len) {
		LOG_ERR(ERR_BAD_OPCODE_LEN, "FC_BEACON", rec_packet->len, expected_len);
		return;
	}
  memcpy(&packet, data, sizeof(packet));
	// when it's a BEACON from anchor from your neighborhood
	if (ADDR_ANCHOR(info->last_src) && hop_cnt == 0) {
    uint8_t default_tree_level = 255;
    SYNC_FindOrCreateNeighbour(info->original_src, default_tree_level);
  }
  // add your trace and send message to sink
  // you can't sent info about beacon to your parent
  // because he doesn't know path to sink yet
  dev_addr_t yourParent = CARRY_ParentAddres();
  bool good_parent = yourParent != 0 && yourParent != info->last_src;
  if (settings.mac.role == RTLS_ANCHOR && good_parent) {
		TransferBeacon(&packet, &rec_packet->hops[0]);
  }
  // accept new device and make autoRoute
  else if (settings.mac.role == RTLS_SINK) {
		dev_addr_t parent = hop_cnt > 0 ? rec_packet->hops[0] : settings.mac.addr;
		if (settings.carry.autoRoute && ADDR_ANCHOR(packet.src_did)) {
      // when parent changed, then log this event
      if (CARRY_ParentSet(packet.src_did, parent) >= 3) {
        int level = CARRY_GetTargetLevel(packet.src_did);
        PRINT_Parent(parent, packet.src_did, level);
      }
		} else if (ADDR_TAG(packet.src_did)) {
			CARRY_TrackTag(packet.src_did, parent);
    }
    // send accept message
    SendDevAccepted(packet.src_did, parent);
  }
	PRINT_Beacon(data, packet.src_did, &rec_packet->hops[0]);
}

void FC_STAT_ASK_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_STAT_ASK);
	FC_STAT_s packet;
	dwt_deviceentcnts_t evnt;
	dwt_readeventcounters(&evnt);
	packet.FC = FC_STAT_RESP;
	packet.len = sizeof(packet);
	packet.tx_cnt = evnt.TXF;
	packet.rx_cnt = evnt.CRCG;
	packet.to_cnt = evnt.SFDTO + evnt.PTO + evnt.RTO;
	packet.err_cnt = evnt.PHE + evnt.RSL + evnt.CRCB + evnt.OVER;
	packet.battery_mV = PORT_BatteryVoltage();
	packet.uptime_ms = PORT_TickMs();
	if (info->original_src == ADDR_BROADCAST) {
		PRINT_Stat(&packet, settings.mac.addr);
	} else {
		BIN_SEND_RESP(FC_STAT_RESP, &packet, packet.len, info);
	}
}

void FC_STAT_RESP_cb(const void* data, const prot_packet_info_t* info) {
	// message is copied to local struct to avoid unaligned access exception
	BIN_ASSERT(*(uint8_t* )data == FC_STAT_RESP);
	FC_STAT_s packet;
	memcpy(&packet, data, sizeof(packet));
	PRINT_Stat(&packet, info->original_src);
}

void FC_VERSION_ASK_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_VERSION_ASK);
	FC_VERSION_s packet;
	packet.FC = FC_VERSION_RESP;
	packet.len = sizeof(packet);
	packet.hMajor = settings_otp->h_major;
	packet.hMinor = settings_otp->h_minor;
	packet.fMajor = settings.version.f_major;
	packet.fMinor = settings.version.f_minor;
	packet.hash = settings.version.f_hash;
	packet.role = settings.mac.role;
	packet.serial = settings_otp->serial;
	if (info->original_src == ADDR_BROADCAST) {
		PRINT_Version(&packet, settings.mac.addr);
	} else {
		BIN_SEND_RESP(FC_VERSION_RESP, &packet, packet.len, info);
	}
}

void FC_VERSION_RESP_cb(const void* data, const prot_packet_info_t* info) {
	// message is copied to local struct to avoid unaligned access exception
	BIN_ASSERT(*(uint8_t* )data == FC_VERSION_RESP);
	FC_VERSION_s packet;
	memcpy(&packet, data, sizeof(packet));
	PRINT_Version(&packet, info->original_src);
}

void FC_DEV_ACCEPTED_cb(const void* data, const prot_packet_info_t* info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t*)data == FC_DEV_ACCEPTED);
  if(settings.mac.role == RTLS_SINK) {
		LOG_WRN(WRN_SINK_ACCEPT_SINK, "Dev accepted from other sink");
	  return;
  }
  FC_DEV_ACCEPTED_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_DeviceAccepted(&packet, info->original_src);
}

void FC_SETTINGS_SAVE_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_SETTINGS_SAVE);
	int ret = SETTINGS_Save();
	FC_SETTINGS_SAVE_RESULT_s packet;
	packet.FC = FC_SETTINGS_SAVE_RESULT;
	packet.len = sizeof(packet);
	packet.result = ret;
	if (info->original_src == ADDR_BROADCAST) {
		PRINT_SettingsSaveResult(&packet, settings.mac.addr);
	} else {
		BIN_SEND_RESP(FC_SETTINGS_SAVE_RESULT, &packet, packet.len, info);
	}
}

void FC_SETTINGS_SAVE_RESULT_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_SETTINGS_SAVE_RESULT);
	FC_SETTINGS_SAVE_RESULT_s packet;
	memcpy(&packet, data, sizeof(packet));
	PRINT_SettingsSaveResult(&packet, info->original_src);
}

void FC_RESET_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_RESET);
	PORT_Reboot();
}

void FC_RFSET_ASK_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_RFSET_ASK);
	FC_RF_SET_s packet;
	packet.FC = FC_RFSET_RESP;
	packet.len = sizeof(packet);
	packet.br = settings.transceiver.dwt_config.dataRate;
	packet.chan = settings.transceiver.dwt_config.chan;
	packet.code = settings.transceiver.dwt_config.txCode;
	packet.ns_sfd = settings.transceiver.dwt_config.nsSFD;
	packet.pac = settings.transceiver.dwt_config.rxPAC;
	packet.plen = settings.transceiver.dwt_config.txPreambLength;
	packet.sfd_to = settings.transceiver.dwt_config.sfdTO;
	packet.smart_tx = settings.transceiver.smart_tx;
	if (info->original_src == ADDR_BROADCAST) {
		PRINT_RFSet(&packet, settings.mac.addr);
	} else {
		BIN_SEND_RESP(FC_RFSET_RESP, &packet, packet.len, info);
	}
}

void FC_RFSET_RESP_cb(const void* data, const prot_packet_info_t* info) {
	// message is copied to local struct to avoid unaligned access exception
	BIN_ASSERT(*(uint8_t* )data == FC_RFSET_RESP);
	FC_RF_SET_s packet;
	memcpy(&packet, data, sizeof(packet));
	PRINT_RFSet(data, info->original_src);
}

void FC_RFSET_SET_cb(const void* data, const prot_packet_info_t* info) {
	// message is copied to local struct to avoid unaligned access exception
	BIN_ASSERT(*(uint8_t* )data == FC_RFSET_SET);
	FC_RF_SET_s packet;
	memcpy(&packet, data, sizeof(packet));
	if (DWT_BR_110K <= packet.br && packet.br <= DWT_BR_6M8) {
		settings.transceiver.dwt_config.dataRate = packet.br;
	}
	if (0 < packet.chan && packet.chan < 8 && packet.chan != 6) {
		settings.transceiver.dwt_config.chan = packet.chan;
	}
	if (0 < packet.code && packet.code < 25) {
		settings.transceiver.dwt_config.rxCode = packet.code;
		settings.transceiver.dwt_config.txCode = packet.code;
	}
	if (packet.ns_sfd == 0 || packet.ns_sfd == 1) {
		settings.transceiver.dwt_config.nsSFD = packet.ns_sfd;
	}
	if (DWT_PAC8 <= packet.pac && packet.pac <= DWT_PAC64) {
		settings.transceiver.dwt_config.rxPAC = packet.pac;
	}
	if (packet.prf == DWT_PRF_16M || packet.prf == DWT_PRF_64M) {
		settings.transceiver.dwt_config.prf = packet.prf;
	}
	if (packet.sfd_to != 0) {
		settings.transceiver.dwt_config.sfdTO = packet.sfd_to;
	}
	if (packet.plen == DWT_PLEN_64 || packet.plen == DWT_PLEN_128 || packet.plen == DWT_PLEN_256
	    || packet.plen == DWT_PLEN_512 || packet.plen == DWT_PLEN_1024 || packet.plen == DWT_PLEN_1536
	    || packet.plen == DWT_PLEN_2048 || packet.plen == DWT_PLEN_4096) {
		settings.transceiver.dwt_config.txPreambLength = packet.plen;
	}
	if (packet.smart_tx != 255) {
		settings.transceiver.smart_tx = packet.smart_tx;
	}
	SETTINGS_Save();
	uint8_t ask_data[] = { FC_RFSET_ASK, 2 };
	FC_RFSET_ASK_cb(&ask_data, info);
	PORT_SleepMs(100);  // to send response
	MAC_Reinit();       // to load new settings into transceiver
}

void FC_RFTXSET_ASK_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_RFTXSET_ASK);
	FC_RF_TX_SET_s packet;
	packet.FC = FC_RFTXSET_RESP;
	packet.len = sizeof(packet);
	packet.power = settings.transceiver.dwt_txconfig.power;
	packet.pg_dly = settings.transceiver.dwt_txconfig.PGdly;
	if (info->original_src == ADDR_BROADCAST) {
		PRINT_RFTxSet(&packet, settings.mac.addr);
	} else {
		BIN_SEND_RESP(FC_RFTXSET_RESP, &packet, packet.len, info);
	}
}

void FC_RFTXSET_RESP_cb(const void* data, const prot_packet_info_t* info) {
	// message is copied to local struct to avoid unaligned access exception
	BIN_ASSERT(*(uint8_t* )data == FC_RFTXSET_RESP);
	FC_RF_TX_SET_s packet;
	memcpy(&packet, data, sizeof(packet));
	PRINT_RFTxSet(&packet, info->original_src);
}

int rftx_set_get_power(uint32_t my_power, uint32_t rec_power, uint8_t mask, int offset) {
	int P = (rec_power >> offset) & 0xFF;
	uint8_t your_mask = 1 << (offset / 8);
	P = ((mask & your_mask) == 0) ? ((my_power >> offset) & 0xFF) : P;
	return P << offset;
}

void FC_RFTXSET_SET_cb(const void* data, const prot_packet_info_t* info) {
	BIN_ASSERT(*(uint8_t* )data == FC_RFTXSET_SET);
	FC_RF_TX_SET_s packet;
	memcpy(&packet, data, sizeof(packet));
	if (packet.power_mask != 0) {
		int my_power = settings.transceiver.dwt_txconfig.power;
		int new_power = 0;
		new_power |= rftx_set_get_power(my_power, packet.power, packet.power_mask, 0);
		new_power |= rftx_set_get_power(my_power, packet.power, packet.power_mask, 8);
		new_power |= rftx_set_get_power(my_power, packet.power, packet.power_mask, 16);
		new_power |= rftx_set_get_power(my_power, packet.power, packet.power_mask, 24);
		settings.transceiver.dwt_txconfig.power = new_power;
	}
	if (packet.pg_dly != 0) {
		settings.transceiver.dwt_txconfig.PGdly = packet.pg_dly;
	}
	SETTINGS_Save();
	uint8_t ask_data[] = { FC_RFTXSET_ASK, 2 };
	FC_RFTXSET_ASK_cb(&ask_data, info);
	PORT_SleepMs(100);  // to send response
	MAC_Reinit();       // to load new settings into transceiver
}

void FC_BLE_ASK_cb(const void* data, const prot_packet_info_t* info) {
BLE_CODE(BIN_ASSERT(*(uint8_t*)data == FC_BLE_ASK); FC_BLE_SET_s packet;
		packet.FC = FC_BLE_RESP; packet.len = sizeof(packet);
		packet.is_enabled = settings.ble.is_enabled;
		packet.tx_power = settings.ble.tx_power;
		if (info->original_src == ADDR_BROADCAST) {
			PRINT_BleSet(&packet, settings.mac.addr);
		} else {BIN_SEND_RESP(FC_BLE_RESP, &packet, packet.len, info);})
}

void FC_BLE_RESP_cb(const void* data, const prot_packet_info_t* info) {
BLE_CODE(
	// message is copied to local struct to avoid unaligned access exception
	BIN_ASSERT(*(uint8_t*)data == FC_BLE_RESP); FC_BLE_SET_s packet;
	memcpy(&packet, data, sizeof(packet));
	PRINT_BleSet(data, info->original_src);)
}

void FC_BLE_SET_cb(const void* data, const prot_packet_info_t* info) {
BLE_CODE(
// message is copied to local struct to avoid unaligned access exception
BIN_ASSERT(*(uint8_t*)data == FC_BLE_SET);
FC_BLE_SET_s packet;
memcpy(&packet, data, sizeof(packet));
if (packet.tx_power != -1) {
	PORT_BleSetPower(packet.tx_power);
}
uint8_t ble_enabled_buf = settings.ble.is_enabled;
if ((int8_t)packet.is_enabled != -1) {
	settings.ble.is_enabled = packet.is_enabled;
}
SETTINGS_Save();
uint8_t ask_data[2];
ask_data[0] = FC_BLE_ASK;
ask_data[1] = 2;
FC_BLE_ASK_cb(&ask_data, info);
PORT_WatchdogRefresh();
PORT_SleepMs(50);  // to send response
PORT_WatchdogRefresh();
if (ble_enabled_buf != settings.ble.is_enabled) {
	PORT_Reboot();
	}
	)
}

void FC_IMU_ASK_cb(const void* data, const prot_packet_info_t* info) {
  BIN_ASSERT(*(uint8_t*)data == FC_IMU_ASK);
  FC_IMU_SET_s packet;
  packet.FC = FC_IMU_RESP;
  packet.len = sizeof(packet);
  packet.is_enabled = settings.imu.is_enabled;
  packet.delay = settings.imu.no_motion_period;
  if (info->original_src == ADDR_BROADCAST) {
    PRINT_ImuSet(&packet, settings.mac.addr);
  } else {
    BIN_SEND_RESP(FC_IMU_RESP, &packet, packet.len, info);
  }
}

void FC_IMU_RESP_cb(const void* data, const prot_packet_info_t* info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t*)data == FC_IMU_RESP);
  FC_IMU_SET_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_ImuSet(data, info->original_src);
}

void FC_IMU_SET_cb(const void* data, const prot_packet_info_t* info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t*)data == FC_IMU_SET);
  FC_IMU_SET_s packet;
  memcpy(&packet, data, sizeof(packet));
  if ((int8_t)packet.delay != -1) {
    PORT_ImuIrqHandler();  // to reset current no-motion time
    settings.imu.no_motion_period = packet.delay;
  }
  if ((int8_t)packet.is_enabled != -1) {
    settings.imu.is_enabled = packet.is_enabled;
    PORT_ImuInit(settings.mac.role == RTLS_TAG);
  }
  uint8_t ask_data[2];
  ask_data[0] = FC_IMU_ASK;
  ask_data[1] = 2;
  FC_IMU_ASK_cb(&ask_data, info);
}

void FC_MAC_ASK_cb(const void* data, const prot_packet_info_t* info) {
  BIN_ASSERT(*(uint8_t*)data == FC_MAC_ASK);
  FC_MAC_SET_s packet;
  packet.FC = FC_MAC_RESP;
  packet.len = sizeof(packet);
  packet.pan = settings.mac.pan;
  packet.addr = settings.mac.addr;
  packet.raport_anchor_to_anchor_distances =
      settings.mac.raport_anchor_anchor_distance;
  packet.role = settings.mac.role;
  packet.beacon_period_ms = settings.mac.beacon_period_ms;
  packet.guard_time_us = settings.mac.slot_guard_time_us;
  packet.slot_period_us = settings.mac.slots_sum_time_us;
  packet.slot_time_us = settings.mac.slot_time_us;
  if (info->original_src == ADDR_BROADCAST) {
    PRINT_MacSet(&packet, settings.mac.addr);
  } else {
    BIN_SEND_RESP(FC_MAC_RESP, &packet, packet.len, info);
  }
}

void FC_MAC_RESP_cb(const void* data, const prot_packet_info_t* info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t*)data == FC_MAC_RESP);
  FC_MAC_SET_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_MacSet(data, info->original_src);
}

static bool MAC_CheckRoleAndAddressMatch(rtls_role role, dev_addr_t addr) {
  if (role == RTLS_DEFAULT || role == RTLS_LISTENER) {
    return true;
  }
if (role == RTLS_TAG && ADDR_TAG(addr)) {
    return true;
  }
if ((role == RTLS_SINK || role == RTLS_ANCHOR) && ADDR_ANCHOR(addr)) {
    return true;
  }
  return false;
}

void FC_MAC_SET_cb(const void* data, const prot_packet_info_t* info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t*)data == FC_MAC_SET);
  FC_MAC_SET_s packet;
  memcpy(&packet, data, sizeof(packet));
  if (packet.beacon_period_ms != UINT32_MAX) {
    settings.mac.beacon_period_ms = packet.beacon_period_ms;
  }
  if (packet.slot_period_us != UINT32_MAX) {
    settings.mac.slots_sum_time_us = packet.slot_period_us;
  }
  if (packet.slot_time_us != UINT32_MAX) {
    settings.mac.slot_time_us = packet.slot_time_us;
  }
  if (packet.guard_time_us != UINT16_MAX) {
    settings.mac.slot_guard_time_us = packet.guard_time_us;
  }
if (packet.raport_anchor_to_anchor_distances < 2 && ADDR_ANCHOR(settings.mac.addr)) {
    settings.mac.raport_anchor_anchor_distance = packet.raport_anchor_to_anchor_distances;
  }
  // sprawdz czy adres jest zgodny z rola
  if (packet.addr != ADDR_BROADCAST && packet.addr != 0) {
    if (packet.role == UINT8_MAX &&
        (packet.addr & ADDR_ANCHOR_FLAG) == (settings.mac.addr & ADDR_ANCHOR_FLAG)) {
      settings.mac.role = packet.role;
    } else if (MAC_CheckRoleAndAddressMatch(packet.role, packet.addr)) {
      settings.mac.role = packet.role;
      settings.mac.addr = packet.addr;
    }
  }
  if (packet.role != UINT8_MAX) {
    if (MAC_CheckRoleAndAddressMatch(packet.role, settings.mac.addr)) {
      settings.mac.role = packet.role;
    }
  }
  if (packet.pan != UINT16_MAX && packet.pan != 0) {
    settings.mac.pan = packet.pan;
  }
  uint8_t ask_data[2];
  ask_data[0] = FC_MAC_ASK;
  ask_data[1] = 2;
  FC_MAC_ASK_cb(&ask_data, info);
  //todo: przydalby sie tutaj opoznony reset
  MAC_Reinit();
}

const prot_cb_t prot_cb_tab[] = {
    ADD_FC(FC_BEACON, FC_BEACON_cb),
    ADD_FC(FC_TURN_ON, FC_TURN_ON_cb),
    ADD_FC(FC_TURN_OFF, FC_TURN_OFF_cb),
    ADD_FC(FC_DEV_ACCEPTED, FC_DEV_ACCEPTED_cb),
    ADD_FC(FC_CARRY, CARRY_ParseMessage),
    ADD_FC(FC_FU, FU_HandleAsDevice),
    ADD_FC(FC_SETTINGS_SAVE, FC_SETTINGS_SAVE_cb),
    ADD_FC(FC_SETTINGS_SAVE_RESULT, FC_SETTINGS_SAVE_RESULT_cb),
    ADD_FC(FC_RESET, FC_RESET_cb),
    ADD_FC(FC_VERSION_ASK, FC_VERSION_ASK_cb),
    ADD_FC(FC_VERSION_RESP, FC_VERSION_RESP_cb),
    ADD_FC(FC_STAT_ASK, FC_STAT_ASK_cb),
    ADD_FC(FC_STAT_RESP, FC_STAT_RESP_cb),
    ADD_FC(FC_RFSET_ASK, FC_RFSET_ASK_cb),
    ADD_FC(FC_RFSET_RESP, FC_RFSET_RESP_cb),
    ADD_FC(FC_RFTXSET_SET, FC_RFTXSET_SET_cb),
    ADD_FC(FC_RFTXSET_ASK, FC_RFTXSET_ASK_cb),
    ADD_FC(FC_RFTXSET_RESP, FC_RFTXSET_RESP_cb),
    ADD_FC(FC_RFSET_SET, FC_RFSET_SET_cb),
    ADD_FC(FC_TOA_INIT, FC_TOA_INIT_cb),
    ADD_FC(FC_TOA_RES, FC_TOA_RES_cb),
    ADD_FC(FC_BLE_ASK, FC_BLE_ASK_cb),
    ADD_FC(FC_BLE_RESP, FC_BLE_RESP_cb),
    ADD_FC(FC_BLE_SET, FC_BLE_SET_cb),
    ADD_FC(FC_IMU_ASK, FC_IMU_ASK_cb),
    ADD_FC(FC_IMU_RESP, FC_IMU_RESP_cb),
    ADD_FC(FC_IMU_SET, FC_IMU_SET_cb),
    ADD_FC(FC_MAC_ASK, FC_MAC_ASK_cb),
    ADD_FC(FC_MAC_RESP, FC_MAC_RESP_cb),
    ADD_FC(FC_MAC_SET, FC_MAC_SET_cb),
    ADD_FC(FC_TDOA_BEACON_TAG_INFO, FC_TDOA_BEACON_TAG_INFO_cb),
    ADD_FC(FC_TDOA_BEACON_ANCHOR_INFO, FC_TDOA_BEACON_ANCHOR_INFO_cb),
};
const int prot_cb_len = sizeof(prot_cb_tab) / sizeof(*prot_cb_tab);
