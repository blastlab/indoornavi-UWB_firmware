#include "transceiver.h"
#include "decadriver/deca_regs.h"

int transceiver_br = 0;
int transceiver_plen = 0;
int transceiver_pac = 0;
int transceiver_sfd = 0;

static void _TRANSCEIVER_FillTxConfig(transceiver_settings_t* ts);
static void _TRANSCEIVER_InitGlobalsFromSet(transceiver_settings_t* ts,
bool set_default_pac);
static int TRANSCEIVER_CalcSfdTo();
static int TRANSCEIVER_CalcPGdly(int ch);

void TRANSCEIVER_Init() {
	int ret;
	dwt_config_t* conf = &settings.transceiver.dwt_config;

	// set global variables correct with transceiver settings and update pac size
	if (settings.transceiver.dwt_txconfig.power == 0) {
		_TRANSCEIVER_FillTxConfig(&settings.transceiver);
		_TRANSCEIVER_InitGlobalsFromSet(&settings.transceiver, 1);
	} else {
		_TRANSCEIVER_InitGlobalsFromSet(&settings.transceiver, 0);
	}

	// check if sfdTO is correct
	if (conf->sfdTO == 0) {
		conf->sfdTO = TRANSCEIVER_CalcSfdTo();
	}
	if (settings.transceiver.dwt_txconfig.PGdly == 0) {
		settings.transceiver.dwt_txconfig.PGdly = TRANSCEIVER_CalcPGdly(conf->chan);
	}

	PORT_SpiSpeedSlow(true);
	ret = dwt_readdevid();

	// clear the sleep bit - so that after the hard reset below
	// the DW does not go into sleep
	if (ret != DWT_DEVICE_ID) {
		PORT_WakeupTransceiver(); // device is asleep
		dwt_softreset();
	}

	// reset device
	PORT_ResetTransceiver();
	PORT_SleepMs(5);

	// set spi low rate and check connection
	PORT_SpiSpeedSlow(true);
	ret = dwt_readdevid();
	TRANSCEIVER_ASSERT(ret == DWT_DEVICE_ID);

	// initialize device and load ucode
	ret = dwt_initialise(DWT_LOADUCODE);
	TRANSCEIVER_ASSERT(ret == DWT_SUCCESS);

	// set spi rate high and check connection
	PORT_SpiSpeedSlow(false);
	ret = dwt_readdevid();
	TRANSCEIVER_ASSERT(ret == DWT_DEVICE_ID);

	// configure channel, prf, br, ...
	dwt_configure(&settings.transceiver.dwt_config);

	// set output power
	dwt_setsmarttxpower(settings.transceiver.smart_tx);
	dwt_configuretxrf(&settings.transceiver.dwt_txconfig);

	// preable == 64 has different Operational Parameter Set
	if (settings.transceiver.dwt_config.txPreambLength == DWT_PLEN_64) {
		dwt_loadopsettabfromotp(DWT_OPSET_64LEN);
	}

	// Apply default antenna delay value. See NOTE 1 below.
	dwt_setrxantennadelay(settings.transceiver.ant_dly_rx);
	dwt_settxantennadelay(settings.transceiver.ant_dly_tx);

	// turn on leds and event counters
	if (settings.transceiver.low_power_mode) {
		dwt_setleds(0);
		dwt_configeventcounters(0);
	} else {
		dwt_setleds(3);
		dwt_configeventcounters(1);
	}

	// turn on default rx mode
	dwt_setrxtimeout(0);
	dwt_setpreambledetecttimeout(0);
}

void TRANSCEIVER_SetCb(dwt_cb_t tx_cb, dwt_cb_t rx_cb, dwt_cb_t rxto_cb, dwt_cb_t rxerr_cb) {
	int tx_flags = SYS_STATUS_ALL_TX | SYS_STATUS_ALL_DBLBUFF;
	int rx_flags = SYS_STATUS_ALL_RX_GOOD;
	int err_flags = SYS_STATUS_ALL_RX_ERR | SYS_STATUS_ALL_RX_TO;
	/*DWT_INT_RPHE |   // receiver PHY header error
	 DWT_INT_RFCE |   // receiver CRC error
	 DWT_INT_RFSL |   // sync lost
	 DWT_INT_RXOVRR | // receiver overrun
	 DWT_INT_RXPTO |  // preamble detection timeout
	 DWT_INT_SFDT |   // start frame delimiter timeout
	 DWT_INT_ARFE |   // frame rejected (due to frame filtering
	 configuration)*/
	// connect interrupts
	dwt_setinterrupt(tx_flags | rx_flags | err_flags, 1);
	dwt_setcallbacks(tx_cb, rx_cb, rxto_cb, rxerr_cb);
}

void TRANSCEIVER_SetAddr(pan_dev_addr_t pan_addr, dev_addr_t dev_addr) {
	dwt_setaddress16(dev_addr);
	dwt_setpanid(pan_addr);
}

void TRANSCEIVER_DefaultRx() {
	dwt_setrxtimeout(0);
	dwt_setpreambledetecttimeout(0);
	dwt_setlowpowerlistening(false);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
	return;
	if (settings.transceiver.low_power_mode) {
		int on_pac = 2;
		int off_time_us = 20;
		TRANSCEIVER_ASSERT(transceiver_plen + transceiver_pac < off_time_us);
		TRANSCEIVER_ASSERT(0 <= off_time_us);
		TRANSCEIVER_ASSERT(off_time_us < 16);
		dwt_setsniffmode(1, on_pac, off_time_us);
	} else {
		dwt_rxenable(DWT_START_RX_IMMEDIATE);
	}
}

void TRANSCEIVER_EnterDeepSleep()  // TODO
{
}

void TRANSCEIVER_EnterSleep() {
	dwt_forcetrxoff();
	dwt_setleds(0);
	dwt_configuresleep(DWT_PRESRV_SLEEP, DWT_WAKE_CS | DWT_SLP_EN);
	dwt_entersleep();
}

void TRANSCEIVER_WakeUp(uint8_t* buf, int len) {
	TRANSCEIVER_ASSERT(len >= 200);
	PORT_SpiSpeedSlow(true);

	// Need to keep chip select line low for at least 500us
	int ret = dwt_spicswakeup(buf, len);
	TRANSCEIVER_ASSERT(ret == DWT_SUCCESS);
	PORT_SpiSpeedSlow(true);

	// wt_configuretxrf(&settings.transceiver.dwt_txconfig);
	dwt_setrxantennadelay(settings.transceiver.ant_dly_rx);
	dwt_settxantennadelay(settings.transceiver.ant_dly_tx);
}

int64_t TRANSCEIVER_GetRxTimestamp(void) {
	uint8_t ts_tab[5];
	int64_t ts = 0;
	int i;
	dwt_readrxtimestamp(ts_tab);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

int64_t TRANSCEIVER_GetTxTimestamp(void) {
	uint8_t ts_tab[5];
	int64_t ts = 0;
	int i;
	dwt_readtxtimestamp(ts_tab);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

int64_t TRANSCEIVER_GetTime() {
	uint8_t ts_tab[5];
	int64_t ts = 0;
	int i;
	dwt_readsystime(ts_tab);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

void TRANSCEIVER_ReadDiagnostic(int16_t* cRSSI, int16_t* cFPP, int16_t* cSNR) {
	dwt_rxdiag_t diag;
	dwt_readdiagnostics(&diag);

	// calculate distance and signal power
	// user manual 4.7
	const float k = 100.0f;  // for centy prefix
	const float A = settings.transceiver.dwt_config.prf == DWT_PRF_64M ? 121.74f : 113.77f;
	const float N = diag.rxPreamCount - transceiver_sfd;
	float RSSItmp;
	RSSItmp = diag.firstPathAmp1 * diag.firstPathAmp1;
	RSSItmp += diag.firstPathAmp2 * diag.firstPathAmp2;
	RSSItmp += diag.firstPathAmp3 * diag.firstPathAmp3;
	RSSItmp /= diag.rxPreamCount;
	if (cFPP != 0) {
		*cFPP = k * 10.0f * log10f(RSSItmp) - k * A;
	}
	if (cRSSI != 0) {
		RSSItmp = (float)diag.maxGrowthCIR * (1 << 17) / (N * N);
		*cRSSI = k * 10.0f * log10f(RSSItmp) - k * A;
	}
	if (cSNR != 0) {
		*cSNR = k * 20.0f * log10f((float)diag.firstPathAmp1 / diag.stdNoise);
	}
}

int TRANSCEIVER_Send(const void* buf, unsigned int len) {
	TRANSCEIVER_ASSERT(buf != 0);
	TRANSCEIVER_ASSERT(len > 6);
	TRANSCEIVER_ASSERT(len < 1024);
	const bool ranging_frame = false;
	dwt_forcetrxoff();
	dwt_writetxdata(len + 2, (uint8_t*)buf, 0);
	dwt_writetxfctrl(len + 2, 0, ranging_frame);
	return dwt_starttx(DWT_START_TX_IMMEDIATE);
}

int TRANSCEIVER_SendRanging(const void* buf, unsigned int len, uint8_t flags) {
	TRANSCEIVER_ASSERT(buf != 0);
	const bool ranging_frame = true;
	dwt_writetxdata(len + 2, (uint8_t*)buf, 0);
	dwt_writetxfctrl(len + 2, 0, ranging_frame);
	return dwt_starttx(flags);
}

void TRANSCEIVER_Read(void* buf, unsigned int len) {
	dwt_readrxdata(buf, len, 0);
}

int TRANSCEIVER_EstimateTxTimeUs(unsigned int len) {
	const uint16_t data_block_size = 330;
	const uint16_t reed_solomon_bits = 48;

	float tx_time = 0.0;
	float symbol_duration = 1000e-9f;

	// Choose the SHR
	// datasheet 5.1.4
	switch (settings.transceiver.dwt_config.prf) {
		case DWT_PRF_16M:
			symbol_duration = 993.59e-9f;
			break;
		case DWT_PRF_64M:
			symbol_duration = 1017.63e-9f;
			break;
	}

	// Find the preamble length
	tx_time += transceiver_plen * symbol_duration;

	// CRC len
	len += 2;

	// Bytes to bits
	len *= 8;

	// Add Reed-Solomon parity bits
	len += reed_solomon_bits * (len + data_block_size - 1) / data_block_size;

	// calc tx time
	tx_time += ((float)len) / transceiver_br;
	return (int)(1e6 * tx_time);
}

static void _TRANSCEIVER_FillTxConfig(transceiver_settings_t* ts) {
	int ch = ts->dwt_config.chan;
	// tx data, depends on channel
	const uint8_t PGdly[] = { 0, 0xC9, 0xC2, 0xC5, 0x95, 0xC0, 0, 0x93 };
	ts->dwt_txconfig.PGdly = PGdly[ch];
	// todo: use power value below maximum
	//  const int TXpower16d[] = {0,          0x75757575, 0x75757575, 0x6F6F6F6F,
	//                            0x5F5F5F5F, 0x48484848, 0,          0x92929292};
	//  const int TXpower64d[] = {0,          0x67676767, 0x67676767, 0x8B8B8B8B,
	//                            0x9A9A9A9A, 0x85858585, 0,          0xD1D1D1D1};
	//  const int TXpower16e[] = {0,          0x15355575, 0x15355575, 0x0F2F4F6F,
	//                            0x1F1F3F5F, 0x0E082848, 0,          0x32527292};
	//  const int TXpower64e[] = {0,          0x07274767, 0x07274767, 0x2B4B6B8B,
	//                            0x3A5A7A9A, 0x25456585, 0,          0x5171B1D1};
	//
	//  // when data rate == 6M8 then smart power en should be enabled
	//  const bool prf_is_16M = ts->dwt_config.prf == DWT_PRF_16M;
	//  if (!ts->smart_tx) {
	//    ts->dwt_txconfig.power = prf_is_16M ? TXpower16d[ch] : TXpower64d[ch];
	//  } else {
	//    ts->dwt_txconfig.power = prf_is_16M ? TXpower16e[ch] : TXpower64e[ch];
	//  }
	ts->dwt_txconfig.power = 0x0E080222;  // todo: uzywac mniejszej mocy default
}

static void _TRANSCEIVER_InitGlobalsFromSet(transceiver_settings_t* ts,
bool set_default_pac) {
	TRANSCEIVER_ASSERT(ts != 0);
	TRANSCEIVER_ASSERT(ts->dwt_config.dataRate < 3);
	TRANSCEIVER_ASSERT(ts->dwt_config.rxPAC < 4 || set_default_pac);

	const int br[] = { 110000, 850000, 6800000 };
	const int sfd_len[] = { 64, 16, 8 };
	const int pac[] = { 8, 16, 32, 64 };
	int def_pac = 0;

	transceiver_br = br[ts->dwt_config.dataRate];
	transceiver_sfd = sfd_len[ts->dwt_config.dataRate];

	switch (ts->dwt_config.txPreambLength) {
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

	if (set_default_pac) {
		ts->dwt_config.rxPAC = def_pac;
	}

	transceiver_pac = pac[ts->dwt_config.rxPAC];
}

static int TRANSCEIVER_CalcSfdTo() {
	TRANSCEIVER_ASSERT(transceiver_sfd > 0);
	TRANSCEIVER_ASSERT(transceiver_plen > 0);
	TRANSCEIVER_ASSERT(transceiver_pac > 0);
	return 1 + transceiver_plen + transceiver_sfd - transceiver_pac;
}

static int TRANSCEIVER_CalcPGdly(int ch) {
	TRANSCEIVER_ASSERT(ch > 0);
	TRANSCEIVER_ASSERT(ch != 6);
	TRANSCEIVER_ASSERT(ch < 8);
	const int PGdly[] = { 0, 0xC9, 0xC2, 0xC5, 0x95, 0xC0, 0, 0x93 };
	return PGdly[ch];
}
