#include "transceiver.h"

int transceiver_br = 0;
int transceiver_plen = 0;
int transceiver_pac = 0;
int transceiver_sfd = 0;

static void _transceiver_fill_txconfig(transceiver_settings_t *ts);
static void _transceiver_init_globals_from_set(transceiver_settings_t *ts,
                                               bool set_default_pac);

int transceiver_init(pan_dev_addr_t pan_addr, dev_addr_t dev_addr)
{
  int ret;

  // set global variables correct with transceiver settings and update pac size
  if (settings.transceiver.dwt_txconfig.power == 0)
  {
    _transceiver_fill_txconfig(&settings.transceiver);
    _transceiver_init_globals_from_set(&settings.transceiver, 1);
  }
  else
  {
    _transceiver_init_globals_from_set(&settings.transceiver, 0);
  }

  // reset device
  reset_DW1000();
  port_sleep_ms(5);

  // set spi low rate and check connection
  spi_speed_slow(true);
  ret = dwt_readdevid();
  TRANSCEIVER_ASSERT(ret == DWT_DEVICE_ID);

  // initialize device and load ucode
  ret = dwt_initialise(DWT_LOADUCODE);
  TRANSCEIVER_ASSERT(ret == DWT_SUCCESS);

  // set spi rate high and check connection
  spi_speed_slow(false);
  ret = dwt_readdevid();
  TRANSCEIVER_ASSERT(ret == DWT_DEVICE_ID);

  // configure channel, prf, br, ...
  dwt_configure(&settings.transceiver.dwt_config);

  // set output power
  dwt_setsmarttxpower(settings.transceiver.dwt_config.dataRate == DWT_BR_6M8);
  dwt_configuretxrf(&settings.transceiver.dwt_txconfig);

  // preable == 64 has different Operational Parameter Set
  if (settings.transceiver.dwt_config.txPreambLength == DWT_PLEN_64)
  {
    dwt_loadopsettabfromotp(DWT_OPSET_64LEN);
  }

  // set device addresses
  dwt_setaddress16(dev_addr);
  dwt_setpanid(pan_addr);

  // Apply default antenna delay value. See NOTE 1 below.
  dwt_setrxantennadelay(settings.transceiver.ant_dly_rx);
  dwt_settxantennadelay(settings.transceiver.ant_dly_tx);

  // turn on leds and event counters
  if (settings.transceiver.low_power_mode)
  {
    dwt_setleds(3);
    dwt_configeventcounters(1);
  }
  else
  {
    dwt_setleds(0);
    dwt_configeventcounters(0);
  }

  // turn on default rx mode
  transceiver_default_rx();

  return 0;
}

void transceiver_set_cb(dwt_cb_t tx_cb, dwt_cb_t rx_cb, dwt_cb_t rxto_cb,
                        dwt_cb_t rxerr_cb)
{
  // connect interrupts
  int isr_flags = DWT_INT_TFRS | // frame send
                  DWT_INT_RFCG | // received frame crc good
                  DWT_INT_RFTO | // receive frame timeout
                  0;
  dwt_setinterrupt(isr_flags, 1);
  dwt_setcallbacks(tx_cb, rx_cb, rxto_cb, rxerr_cb);
}

void transceiver_default_rx()
{
  if (settings.transceiver.low_power_mode)
  {
  }
  else
  {
  }
}

void transceiver_enter_deep_sleep()
{
  dwt_forcetrxoff();
  dwt_setleds(0);
  dwt_configuresleep(DWT_PRESRV_SLEEP, DWT_WAKE_CS | DWT_SLP_EN);
  dwt_entersleep();
}

void transceiver_wake_up(uint8_t *buf, int len)
{
  TRANSCEIVER_ASSERT(len >= 200);
  spi_speed_slow(true);

  // Need to keep chip select line low for at least 500us
  int ret = dwt_spicswakeup(buf, len);
  TRANSCEIVER_ASSERT(ret == DWT_SUCCESS);
  spi_speed_slow(true);

  //wt_configuretxrf(&settings.transceiver.dwt_txconfig);
  dwt_setrxantennadelay(settings.transceiver.ant_dly_rx);
  dwt_settxantennadelay(settings.transceiver.ant_dly_tx);
}

int64_t transceiver_get_rx_timestamp(void)
{
  uint8_t ts_tab[5];
  int64_t ts = 0;
  int i;
  dwt_readrxtimestamp(ts_tab);
  for (i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  return ts;
}

int64_t transceiver_get_tx_timestamp(void)
{
  uint8_t ts_tab[5];
  int64_t ts = 0;
  int i;
  dwt_readtxtimestamp(ts_tab);
  for (i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  return ts;
}

int64_t transceiver_get_time()
{
  uint8_t ts_tab[5];
  int64_t ts = 0;
  int i;
  dwt_readsystime(ts_tab);
  for (i = 4; i >= 0; i--)
  {
    ts <<= 8;
    ts |= ts_tab[i];
  }
  return ts;
}

void transceiver_read_diagnostic(int16_t *cRSSI, int16_t *cFPP, int16_t *cSNR)
{
  dwt_rxdiag_t diag;
  dwt_readdiagnostics(&diag);

  // calculate distance and signal power
  // user manual 4.7
  const float k = 100.0f; // for centy prefix
  const float A =
      settings.transceiver.dwt_config.prf == DWT_PRF_64M ? 121.74f : 113.77f;
  const float N = diag.rxPreamCount - transceiver_sfd;
  float RSSItmp;
  RSSItmp = diag.firstPathAmp1 * diag.firstPathAmp1;
  RSSItmp += diag.firstPathAmp2 * diag.firstPathAmp2;
  RSSItmp += diag.firstPathAmp3 * diag.firstPathAmp3;
  RSSItmp /= diag.rxPreamCount;
  if (cFPP != 0)
  {
    *cFPP = k * 10.0f * log10f(RSSItmp) - k * A;
  }
  if (cRSSI != 0)
  {
    RSSItmp = (float)diag.maxGrowthCIR * (1 << 17) / (N * N);
    *cRSSI = k * 10.0f * log10f(RSSItmp) - k * A;
  }
  if (cSNR != 0)
  {
    *cSNR = k * 20.0f * log10f((float)diag.firstPathAmp1 / diag.stdNoise);
  }
}

int transceiver_send(const void *buf, unsigned int len)
{
  TRANSCEIVER_ASSERT(buf != 0);
  const bool ranging_frame = false;
  dwt_forcetrxoff();
  dwt_writetxdata(len + 2, (uint8_t *)buf, 0);
  dwt_writetxfctrl(len + 2, 0, ranging_frame);
  return dwt_starttx(DWT_START_TX_IMMEDIATE);
}

int transceiver_send_ranging(const void *buf, unsigned int len, uint8_t flags)
{
  TRANSCEIVER_ASSERT(buf != 0);
  const bool ranging_frame = true;
  dwt_forcetrxoff();
  dwt_writetxdata(len + 2, (uint8_t *)buf, 0);
  dwt_writetxfctrl(len + 2, 0, ranging_frame);
  return dwt_starttx(flags);
}

void transceiver_read(void *buf, unsigned int len)
{
  dwt_readrxdata(buf, len, 0);
}

int transceiver_estimate_tx_time_us(unsigned int len)
{
  const uint16_t data_block_size = 330;
  const uint16_t reed_solomon_bits = 48;

  float tx_time = 0.0;
  float symbol_duration = 1000e-9f;

  // Choose the SHR
  // datasheet 5.1.4
  switch (settings.transceiver.dwt_config.prf)
  {
  case DWT_PRF_16M:
    symbol_duration = 993.59e-9f;
    break;
  case DWT_PRF_64M:
    symbol_duration = 1017.63e-9f;
    break;
  }

  // Find the preamble length
  tx_time += transceiver_plen * symbol_duration;

  // Bytes to bits
  len *= 8;

  // Add Reed-Solomon parity bits
  len += reed_solomon_bits * (len + data_block_size - 1) / data_block_size;

  // calc tx time
  tx_time += ((float)len) / transceiver_br;
  return (int)(1e6 * tx_time);
}

static void _transceiver_fill_txconfig(transceiver_settings_t *ts)
{
  int ch = ts->dwt_config.chan;
  // tx data, depends on channel
  const uint8_t PGdly[] = {0, 0xC9, 0xC2, 0xC5, 0x95, 0xC0, 0, 0x93};
  const int TXpower16d[] = {0, 0x75757575, 0x75757575, 0x6F6F6F6F,
                            0x5F5F5F5F, 0x48484848, 0, 0x92929292};
  const int TXpower64d[] = {0, 0x67676767, 0x67676767, 0x8B8B8B8B,
                            0x9A9A9A9A, 0x85858585, 0, 0xD1D1D1D1};
  const int TXpower16e[] = {0, 0x15355575, 0x15355575, 0x0F2F4F6F,
                            0x1F1F3F5F, 0x0E082848, 0, 0x32527292};
  const int TXpower64e[] = {0, 0x07274767, 0x07274767, 0x2B4B6B8B,
                            0x3A5A7A9A, 0x25456585, 0, 0x5171B1D1};

  ts->dwt_txconfig.PGdly = PGdly[ch];
  // when data rate == 6M8 then smart power en should be enabled
  const bool prf_is_16M = ts->dwt_config.prf == DWT_PRF_16M;
  if (ts->dwt_config.dataRate == DWT_BR_6M8)
  {
    ts->dwt_txconfig.power = prf_is_16M ? TXpower16d[ch] : TXpower64d[ch];
  }
  else
  {
    ts->dwt_txconfig.power = prf_is_16M ? TXpower16e[ch] : TXpower64e[ch];
  }
}

static void _transceiver_init_globals_from_set(transceiver_settings_t *ts,
                                               bool set_default_pac)
{
  TRANSCEIVER_ASSERT(ts != 0);
  TRANSCEIVER_ASSERT(ts->dwt_config.dataRate < 3);
  TRANSCEIVER_ASSERT(ts->dwt_config.rxPAC < 4 || set_default_pac);

  const int br[] = {110000, 850000, 6800000};
  const int sfd_len[] = {64, 16, 8};
  const int pac[] = {8, 16, 32, 64};
  int def_pac = 0;

  transceiver_br = br[ts->dwt_config.dataRate];
  transceiver_sfd = sfd_len[ts->dwt_config.dataRate];

  switch (ts->dwt_config.txPreambLength)
  {
  case DWT_PLEN_64:
    transceiver_plen = 64;
    def_pac = DWT_PAC8;
    break;
  case DWT_PLEN_128:
    transceiver_plen = 128;
    def_pac = DWT_PAC8;
    break;
  case DWT_PLEN_256:
    transceiver_plen = 256;
    def_pac = DWT_PAC16;
    break;
  case DWT_PLEN_512:
    transceiver_plen = 512;
    def_pac = DWT_PAC16;
    break;
  case DWT_PLEN_1024:
    transceiver_plen = 1024;
    def_pac = DWT_PAC32;
    break;
  case DWT_PLEN_1536:
    transceiver_plen = 1536;
    def_pac = DWT_PAC32;
    break;
  case DWT_PLEN_2048:
    transceiver_plen = 2048;
    def_pac = DWT_PAC64;
    break;
  case DWT_PLEN_4096:
    transceiver_plen = 4096;
    def_pac = DWT_PAC64;
    break;
  }

  if (set_default_pac)
  {
    ts->dwt_config.rxPAC = def_pac;
  }

  transceiver_pac = pac[ts->dwt_config.rxPAC];
}

void deca_sleep(unsigned int time_ms) { port_sleep_ms(time_ms); }
