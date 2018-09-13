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
  buf = CARRY_PrepareBufTo(target, &carry);
  if (buf != 0) {
    CARRY_Write(carry, buf, &acc, acc.len);
    CARRY_Send(buf, true);
  }
}

static void TransferBeacon(FC_BEACON_s* packet) {
	mac_buf_t* buf;
	FC_CARRY_s* carry;
	buf = CARRY_PrepareBufTo(CARRY_ParentAddres(), &carry);
	if (buf != 0) {
		packet->len += sizeof(dev_addr_t);
		packet->hop_cnt += 1;
		CARRY_Write(carry, buf, &packet, sizeof(packet));
		CARRY_Write(carry, buf, (uint8_t*)packet + sizeof(FC_BEACON_s),
		            sizeof(dev_addr_t) * (packet->hop_cnt - 1));
		CARRY_Write(carry, buf, &settings.mac.addr, sizeof(dev_addr_t));
		CARRY_Send(buf, false);
	} else {
		LOG_WRN("BEACON parser not enough buffers");
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
	} else {
		LOG_WRN("Not enough buffer to send bin resp");
	}
}

void FC_TURN_ON_cb(const void* data, const prot_packet_info_t* info) {
  BIN_ASSERT(*(uint8_t*)data == FC_TURN_ON);
  FC_TURN_ON_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_TurnOn(data, info->original_src);
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
  BIN_ASSERT(*(uint8_t*)data == FC_BEACON);
  memcpy(&packet, data, sizeof(packet));
  if ((info->last_src & ADDR_ANCHOR_FLAG) && packet.hop_cnt == 0) {
    uint8_t default_tree_level = 255;
    SYNC_FindOrCreateNeighbour(info->original_src, default_tree_level);
  }
  // add your trace and send message to sink
  // you can't sent info about beacon to your parent
  // because he doesn't know path to sink yet
  dev_addr_t yourParent = CARRY_ParentAddres();
  bool good_parent = yourParent != 0 && yourParent != info->last_src;
  if (settings.mac.role == RTLS_ANCHOR && good_parent) {
    TransferBeacon(&packet);
  }
  // accept new device and make autoRoute
  else if (settings.mac.role == RTLS_SINK) {
    dev_addr_t hooped_parent = packet.hops[packet.hop_cnt - 1];
    dev_addr_t parent = packet.hop_cnt > 0 ? hooped_parent : settings.mac.addr;
    if (settings.carry.autoRoute && (packet.src_did & ADDR_ANCHOR_FLAG) != 0) {
      // when parent changed, then log this event
      if (CARRY_ParentSet(packet.src_did, parent) >= 3) {
        int level = CARRY_GetTargetLevel(packet.src_did);
        PRINT_Parent(parent, packet.src_did, level);
      }
    }
    // send accept message
    SendDevAccepted(packet.src_did, parent);
  }
  PRINT_Beacon(data, packet.src_did);
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
	  LOG_WRN("Dev accepted from other sink");
	  return;
  }
  FC_DEV_ACCEPTED_s packet;
  memcpy(&packet, data, sizeof(packet));
  CARRY_SetYourParent(packet.newParent);
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
	packet.pg_dly = settings.transceiver.dwt_txconfig.PGdly;
	packet.plen = settings.transceiver.dwt_config.txPreambLength;
	packet.sfd_to = settings.transceiver.dwt_config.sfdTO;
	packet.power = settings.transceiver.dwt_txconfig.power;
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
	if (packet.power != 0) {
		settings.transceiver.dwt_txconfig.power = packet.power;
	}
	if (packet.pg_dly != 0) {
		settings.transceiver.dwt_txconfig.PGdly = packet.pg_dly;
	}
	SETTINGS_Save();
	uint8_t ask_data[] = { FC_RFSET_ASK, 2 };
	FC_RFSET_ASK_cb(&ask_data, info);
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
BIN_ASSERT(*(uint8_t*)data == FC_BLE_SET); FC_BLE_SET_s packet;
memcpy(&packet, data, sizeof(packet)); if (packet.tx_power != -1) {
	PORT_BleSetPower(packet.tx_power);
}uint8_t ble_enabled_buf = settings.ble.is_enabled;
if ((int8_t)packet.is_enabled != -1) {
	settings.ble.is_enabled = packet.is_enabled;
}SETTINGS_Save();
uint8_t ask_data[2]; ask_data[0] = FC_BLE_ASK; ask_data[1] = 2;
FC_BLE_ASK_cb(&ask_data, info); PORT_WatchdogRefresh();
PORT_SleepMs(50);  // to send response
PORT_WatchdogRefresh();
if (ble_enabled_buf != settings.ble.is_enabled) {PORT_Reboot();})
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
	PORT_ImuIrqHandler();  // to reset current no-motion time
    settings.imu.is_enabled = packet.is_enabled;
  }
  uint8_t ask_data[2];
  ask_data[0] = FC_IMU_ASK;
  ask_data[1] = 2;
  FC_IMU_ASK_cb(&ask_data, info);
}

const prot_cb_t prot_cb_tab[] = {
    {FC_BEACON, FC_BEACON_cb},
    {FC_TURN_ON, FC_TURN_ON_cb},
    {FC_TURN_OFF, FC_TURN_OFF_cb},
    {FC_DEV_ACCEPTED, FC_DEV_ACCEPTED_cb},
    {FC_CARRY, CARRY_ParseMessage},
    {FC_FU, FU_HandleAsDevice},
    {FC_SETTINGS_SAVE, FC_SETTINGS_SAVE_cb},
    {FC_SETTINGS_SAVE_RESULT, FC_SETTINGS_SAVE_RESULT_cb},
    {FC_RESET, FC_RESET_cb},
    {FC_VERSION_ASK, FC_VERSION_ASK_cb},
    {FC_VERSION_RESP, FC_VERSION_RESP_cb},
    {FC_STAT_ASK, FC_STAT_ASK_cb},
    {FC_STAT_RESP, FC_STAT_RESP_cb},
    {FC_RFSET_ASK, FC_RFSET_ASK_cb},
    {FC_RFSET_RESP, FC_RFSET_RESP_cb},
    {FC_RFSET_SET, FC_RFSET_SET_cb},
    {FC_TOA_INIT, FC_TOA_INIT_cb},
    {FC_TOA_RES, FC_TOA_RES_cb},
    {FC_BLE_ASK, FC_BLE_ASK_cb},
    {FC_BLE_RESP, FC_BLE_RESP_cb},
    {FC_BLE_SET, FC_BLE_SET_cb},
    {FC_IMU_ASK, FC_IMU_ASK_cb},
    {FC_IMU_RESP, FC_IMU_RESP_cb},
    {FC_IMU_SET, FC_IMU_SET_cb},
};
const int prot_cb_len = sizeof(prot_cb_tab) / sizeof(*prot_cb_tab);
