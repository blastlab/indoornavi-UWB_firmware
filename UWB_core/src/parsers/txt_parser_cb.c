#include "txt_parser.h"

/**
 * @brief handle message locally or send it to other device
 * depending on info->direct_src address.
 * 
 * @param buf buffer to handle, point at FC field
 * @param info packet extra informations
 */
static void _TXT_Finalize(const void *buf, const prot_packet_info_t *info)
{
  prot_packet_info_t new_info;
  if (info->direct_src == ADDR_BROADCAST)
  {
    memset(&new_info, 0, sizeof(new_info));
    new_info.direct_src = settings.mac.addr;
    BIN_ParseSingle(buf, info);
  }
  else
  {
    mac_buf_t *mbuf = CARRY_PrepareBufTo(info->direct_src);
    if(mbuf != 0) {
      uint8_t *ibuf = (uint8_t*)buf;
      MAC_Write(mbuf, buf, ibuf[1]); // length is always second byte of frame
      MAC_Send(mbuf, true);
    }
    else
    {
      LOG_WRN("Not enough buffers to send frame, FC:%X", *(uint8_t*)buf);
    }
  }
}

/**
 * @brief fully handle ASK type messages without extra parameters
 * 
 * @param info extra packet informations
 * @param FC detailed ask function code descriptor
 */
static void _TXT_Ask(const prot_packet_info_t *info, FC_t FC)
{
  uint8_t buf[2] = {FC, 2};
  _TXT_Finalize(buf, info);
}

// === callbacks ===

static void TXT_StatCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  _TXT_Ask(info, FC_STAT_ASK);
}

static void TXT_VersionCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  _TXT_Ask(info, FC_VERSION_ASK);
}

static void TXT_HangCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  LOG_DBG("HANG");
  while (1)
  {
  }
}

static bool _RFSet_ValidateAndTranslateA(int *ch, int *br, int *plen, int *prf)
{
  bool some_change = false;

  if(*ch >= 0)
  {
    if(!(0 < *ch && *ch < 8 && *ch != 6))
    {
      LOG_ERR("rfset ch 1..7 (without 6)");
      *ch = -1;
    }
    some_change = true;
  }
  if(*br >= 0)
  {
    if(*br == 110)
    {
      *br = DWT_BR_110K;
    } else if(*br == 850)
    {
      *br = DWT_BR_850K;
    } else if(*br == 6800)
    {
      *br = DWT_BR_6M8;
    } else
    {
      LOG_ERR("rfset br 110/850/6800");
      *br = -1;
    }
    some_change = true;
  }
  if(*plen >= 0)
  {
      if (*plen == 64)
        *plen = DWT_PLEN_64; // standard
      else if (*plen == 128)
        *plen = DWT_PLEN_128;
      else if (*plen == 256)
        *plen = DWT_PLEN_256;
      else if (*plen == 512)
        *plen = DWT_PLEN_512;
      else if (*plen == 1024)
        *plen = DWT_PLEN_1024; // standard
      else if (*plen == 1536)
        *plen = DWT_PLEN_1536;
      else if (*plen == 2048)
        *plen = DWT_PLEN_2048;
      else if (*plen == 4096)
        *plen = DWT_PLEN_4096; // standard
      else {
        LOG_ERR("rfset plen 64/128/256/512/1024/1536/2048/4096");
        *plen = -1;
      }
    some_change = true;
  }
  if(*prf >= 0)
  {
    if(*prf == 64)
      *prf = DWT_PRF_64M;
    else if(*prf == 16)
      *prf = DWT_PRF_16M;
    else
    {
      LOG_ERR("rfset prf 16/64");
      *prf = -1;
    }
    some_change = true;
  }
  return some_change;
}

static bool _RFSet_ValidateAndTranslateB(int *pac, int *code, int *nssfd, int *power)
{
  bool some_change = false;
  if(*pac >= 0)
  {
    if(*pac == 8)
      *pac = DWT_PAC8;
    else if(*pac == 16)
      *pac = DWT_PAC16;
    else if(*pac == 32)
      *pac = DWT_PAC32;
    else if(*pac == 64)
      *pac = DWT_PAC64;
    else
    {
      LOG_ERR("rfset pac 8/16/32/64");
      *pac = -1;
    }
    some_change = true;
  }
  if(*code >= 0 && !(0 < *code && *code < 25))
  {
    LOG_ERR("rfset code 1..24");
    *code = -1;
    some_change = true;
  }
  if(*nssfd >= 0 && !(*nssfd == 0 || *nssfd == 1))
  {
    LOG_ERR("rfset nssfd 0/1");
    some_change = true;
  }
  if(*power != -1)
  {
    some_change = true;
  }
  return some_change;
}

// todo: przeniesc obsluge zadania do parse bin poprzez _TXT_ASK
static void TXT_RFSetCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
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

static void TXT_TestCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  LOG_TEST("PASS");
}

static void TXT_SaveCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  SETTINGS_Save();
}

static void TXT_ResetCb(const txt_buf_t *buf, const prot_packet_info_t *info)
{
  PORT_Reboot();
}

const txt_cb_t txt_cb_tab[] = {{"stat", TXT_StatCb},
                               {"version", TXT_VersionCb},
                               {"_hang", TXT_HangCb},
                               {"rfset", TXT_RFSetCb},
                               {"test", TXT_TestCb},
                               {"save", TXT_SaveCb},
                               {"reset", TXT_ResetCb},
                               };

const int txt_cb_len = sizeof(txt_cb_tab) / sizeof(*txt_cb_tab);
