#include "txt_parser.h"

void _TXT_Finalize(mac_buf_t *buf, const prot_packet_info_t *info)
{
  if (info->direct_src == ADDR_BROADCAST)
  {
    buf->dPtr = &buf->buf[0];
    BIN_ParseSingle(buf->dPtr, info);
    MAC_Free(buf);
  }
  else
  {
    MAC_Send(buf, true);
  }
}

void _TXT_Ask(const prot_packet_info_t *info, uint8_t FC)
{
  dev_addr_t addr = info->direct_src;
  mac_buf_t *buf;

  if(addr == ADDR_BROADCAST){
  	buf = MAC_Buffer();
  } else {
  	buf = CARRY_PrepareBufTo(addr);
  }

  if (buf != 0)
  {
    MAC_Write8(buf, FC);
    MAC_Write8(buf, 2); // len
    _TXT_Finalize(buf, info);
  }
}

// === callbacks ===

void TXT_StatCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  _TXT_Ask(info, FC_STAT_ASK);
}

void TXT_VersionCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  _TXT_Ask(info, FC_VERSION_ASK);
}

void TXT_HangCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  while (1)
  {
  }
}

// todo: przeniesc obsluge zadania do parse bin poprzez _TXT_ASK
void TXT_RFSet(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  extern int transceiver_pac, transceiver_plen, transceiver_br;
  const int f[8] = {0, 3494, 3994, 4493, 3994, 6490, 0, 6490};
  const int bw[8] = {0, 499, 499, 499, 1331, 499, 0, 1082};
  dwt_config_t *conf = &settings.transceiver.dwt_config;
  int prf = (conf->prf == DWT_PRF_16M) ? 16 : 64;
  int pac = transceiver_pac;
  int plen = transceiver_plen;
  int dr = transceiver_br;
  LOG_INF("ch:%d-%d/%d dr:%d plen:%d prf:%d pac:%d code:%d nsSfd:%d sfdTo:%d",
          conf->chan, f[conf->chan], bw[conf->chan],
          dr, plen, prf, pac, conf->rxCode, conf->nsSFD, conf->sfdTO);
}

void TXT_TestCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  LOG_TEST("PASS");
}

const txt_cb_t txt_cb_tab[] = {{"stat", TXT_StatCb},
                               {"version", TXT_VersionCb},
                               {"_hang", TXT_HangCb},
                               {"rfset", TXT_RFSet},
                               {"test", TXT_TestCb}};

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);
