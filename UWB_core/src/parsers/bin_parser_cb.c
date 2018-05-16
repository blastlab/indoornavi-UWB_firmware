#include "../mac/carry.h"
#include "bin_parser.h"
#include "bin_struct.h"
#include "printer.h"


void BIN_SEND_RESP(uint8_t FC, const void *data, uint8_t len,
                   const prot_packet_info_t *info) {
	uint8_t *header = (uint8_t *)data;
	header[0] = FC;
	header[1] = len;
	mac_buf_t *buf = CARRY_PrepareBufTo(info->direct_src);
	MAC_Write(buf, data, len);
	MAC_Send(buf, false);
}

void FC_TURN_ON_cb(const void *data, const prot_packet_info_t *info) {
	LOG_DBG("Device turn on %X", info->direct_src);
}

void FC_BEACON_cb(const void *data, const prot_packet_info_t *info) {
	LOG_DBG("Beacon from %X", info->direct_src);
	if(info->direct_src & ADDR_ANCHOR_FLAG) {
		uint8_t default_tree_level = 255;
		SYNC_FindOrCreateNeightbour(info->direct_src, default_tree_level);
	}
}

void FC_STAT_ASK_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_STAT_ASK);
  FC_STAT_s packet;
  packet.err_cnt = 1;
  packet.rx_cnt = 2;
  if(info->direct_src == ADDR_BROADCAST) {
  	PRINT_Stat(&packet, settings.mac.addr);
  } else {
    BIN_SEND_RESP(FC_STAT_RESP, &packet, packet.len, info);
  }
}

void FC_VERSION_ASK_cb(const void *data, const prot_packet_info_t *info) {
  BIN_ASSERT(*(uint8_t *)data == FC_VERSION_ASK);
  FC_VERSION_s packet;
  packet.hMajor = __H_MAJOR__;
  packet.hMinor = __H_MINOR__;
  packet.fMajor = __F_MAJOR__;
  packet.fMinor = __F_MINOR__;
  packet.hash = __F_HASH__;
  packet.role = settings.mac.role;
  if(info->direct_src == ADDR_BROADCAST) {
  	PRINT_Version(&packet, settings.mac.addr);
  } else {
    BIN_SEND_RESP(FC_STAT_RESP, &packet, packet.len, info);
  }
}

const prot_cb_t prot_cb_tab[] = {
		{FC_TURN_ON, FC_TURN_ON_cb},
		{FC_BEACON, FC_BEACON_cb},
    {FC_CARRY, 0},
    {FC_STAT_ASK, FC_STAT_ASK_cb},
		{FC_VERSION_ASK, FC_VERSION_ASK_cb},
};
const int prot_cb_len = sizeof(prot_cb_tab) / sizeof(*prot_cb_tab);
