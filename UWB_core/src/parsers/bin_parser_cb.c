#include "../mac/carry.h"
#include "bin_parser.h"
#include "bin_struct.h"
#include "printer.h"
#include "../mac/toa_routine.h"
#include "../prot/FU.h"


void BIN_SEND_RESP(FC_t FC, const void *data, uint8_t len,
                   const prot_packet_info_t *info) {
  uint8_t *header = (uint8_t *)data;
  header[0] = (uint8_t)FC;
  header[1] = len;
  FC_CARRY_s* carry;
  mac_buf_t *buf = CARRY_PrepareBufTo(info->direct_src, &carry);
  if(buf != 0) {
	  CARRY_Write(carry, buf, data, len);
	  CARRY_Send(buf, false);
  } else {
	  LOG_WRN("Not enough buffer to send bin resp");
  }
}

void FC_TURN_ON_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_TURN_ON);
  PRINT_TurnOn(data, info->direct_src);
}

void FC_TURN_OFF_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_TURN_OFF);
  PRINT_TurnOff(data, info->direct_src);
}

void FC_BEACON_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_BEACON);
  PRINT_Beacon(data, info->direct_src);
  if(info->direct_src & ADDR_ANCHOR_FLAG) {
    uint8_t default_tree_level = 255;
    SYNC_FindOrCreateNeighbour(info->direct_src, default_tree_level);
  }
  FC_BEACON_s packet = *(FC_BEACON_s*)data;
  packet.len += sizeof(dev_addr_t);
  packet.hop_cnt += 1;
  if(CARRY_ParentAddres() != 0) {
    FC_CARRY_s* carry;
	  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ParentAddres(), &carry);
	  if(buf != 0) {
		  CARRY_Write(carry, buf, &packet, sizeof(packet));
		  CARRY_Write(carry, buf, (uint8_t*)data + sizeof(FC_BEACON_s), sizeof(dev_addr_t) * (packet.hop_cnt-1));
		  CARRY_Write(carry, buf, &settings.mac.addr, sizeof(dev_addr_t));
		  CARRY_Send(buf, false);
	  } else {
		  LOG_WRN("BEACON parser not enough buffers");
	  }
  }
}


void FC_STAT_ASK_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_STAT_ASK);
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
  if(info->direct_src == ADDR_BROADCAST) {
    PRINT_Stat(&packet, settings.mac.addr);
  } else {
    BIN_SEND_RESP(FC_STAT_RESP, &packet, packet.len, info);
  }
}

void FC_STAT_RESP_cb(const void *data, const prot_packet_info_t *info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t *)data == FC_STAT_RESP);
  FC_STAT_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_Stat(&packet, info->direct_src);
}

void FC_VERSION_ASK_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_VERSION_ASK);
  FC_VERSION_s packet;
  packet.FC = FC_VERSION_RESP;
  packet.len = sizeof(packet);
  packet.hMajor = __H_MAJOR__;
  packet.hMinor = __H_MINOR__;
  packet.fMajor = __F_MAJOR__;
  packet.fMinor = __F_MINOR__;
  packet.hash = __F_HASH__;
  packet.role = settings.mac.role;
  if(info->direct_src == ADDR_BROADCAST) {
    PRINT_Version(&packet, settings.mac.addr);
  } else {
    BIN_SEND_RESP(FC_VERSION_RESP, &packet, packet.len, info);
  }
}

void FC_VERSION_RESP_cb(const void *data, const prot_packet_info_t *info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t *)data == FC_VERSION_RESP);
  FC_VERSION_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_Version(&packet, info->direct_src);
}

void FC_DEV_ACCEPTED_cb(const void *data, const prot_packet_info_t *info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t *)data == FC_DEV_ACCEPTED);
  FC_DEV_ACCEPTED_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_DeviceAccepted(&packet, info->direct_src);
}

void FC_RFSET_ASK_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_RFSET_ASK);
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
  if(info->direct_src == ADDR_BROADCAST) {
    PRINT_RFSet(&packet, settings.mac.addr);
  } else {
    BIN_SEND_RESP(FC_RFSET_RESP, &packet, packet.len, info);
  }
}

void FC_RFSET_RESP_cb(const void *data, const prot_packet_info_t *info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t *)data == FC_RFSET_RESP);
  FC_RF_SET_s packet;
  memcpy(&packet, data, sizeof(packet));
  PRINT_RFSet(data, info->direct_src);
}

void FC_RFSET_SET_cb(const void *data, const prot_packet_info_t *info) {
  // message is copied to local struct to avoid unaligned access exception
  BIN_ASSERT(*(uint8_t *)data == FC_RFSET_SET);
  FC_RF_SET_s packet;
  memcpy(&packet, data, sizeof(packet));
  if(DWT_BR_110K <= packet.br && packet.br <= DWT_BR_6M8)
  {
    settings.transceiver.dwt_config.dataRate = packet.br;
  }
  if(0 < packet.chan && packet.chan < 8 && packet.chan != 6)
  {
    settings.transceiver.dwt_config.chan = packet.chan;
  }
  if(0 < packet.code && packet.code < 25)
  {
    settings.transceiver.dwt_config.rxCode = packet.code;
    settings.transceiver.dwt_config.txCode = packet.code;
  }
  if(packet.ns_sfd == 0 || packet.ns_sfd == 1)
  {
    settings.transceiver.dwt_config.nsSFD = packet.ns_sfd;
  }
  if(DWT_PAC8 <= packet.pac && packet.pac <= DWT_PAC64)
  {
    settings.transceiver.dwt_config.rxPAC = packet.pac;
  }
  if(packet.prf == DWT_PRF_16M || packet.prf == DWT_PRF_64M)
  {
    settings.transceiver.dwt_config.prf = packet.prf;
  }
  if(packet.sfd_to != 0)
  {
    settings.transceiver.dwt_config.sfdTO = packet.sfd_to;
  }
  if(packet.plen == DWT_PLEN_64 || packet.plen == DWT_PLEN_128 || packet.plen == DWT_PLEN_256 ||
      packet.plen == DWT_PLEN_512 || packet.plen == DWT_PLEN_1024 || packet.plen == DWT_PLEN_1536 ||
      packet.plen == DWT_PLEN_2048 || packet.plen == DWT_PLEN_4096)
  {
    settings.transceiver.dwt_config.txPreambLength = packet.plen;
  }
  if(packet.power != 0)
  {
    settings.transceiver.dwt_txconfig.power = packet.power;
  }
  if(packet.pg_dly != 0)
  {
    settings.transceiver.dwt_txconfig.PGdly = packet.pg_dly;
  }
  SETTINGS_Save();
  uint8_t ask_data[] = {FC_RFSET_ASK, 2};
  FC_RFSET_ASK_cb(&ask_data, info);
  PORT_SleepMs(100);  // to send response
  MAC_Reinit(); // to load new settings into transceiver
}

const prot_cb_t prot_cb_tab[] = {
    {FC_BEACON, FC_BEACON_cb},
    {FC_TURN_ON, FC_TURN_ON_cb},
    {FC_TURN_OFF, FC_TURN_OFF_cb},
    {FC_DEV_ACCEPTED, FC_DEV_ACCEPTED_cb},
    {FC_CARRY, CARRY_ParseMessage}, //this code has different argumetns
    {FC_FU, FU_HandleAsDevice},
    {FC_VERSION_ASK, FC_VERSION_ASK_cb},
    {FC_VERSION_RESP, FC_VERSION_RESP_cb},
    {FC_STAT_ASK, FC_STAT_ASK_cb},
    {FC_STAT_RESP, FC_STAT_RESP_cb},
    {FC_RFSET_ASK, FC_RFSET_ASK_cb},
    {FC_RFSET_RESP, FC_RFSET_RESP_cb},
    {FC_RFSET_SET, FC_RFSET_SET_cb},
    {FC_TOA_INIT, FC_TOA_INIT_cb},
};
const int prot_cb_len = sizeof(prot_cb_tab) / sizeof(*prot_cb_tab);
