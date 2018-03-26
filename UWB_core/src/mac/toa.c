#include "toa.h"

// private function from calib file
int _toa_get_range_bias(uint8 chan, int range, uint8 prf, int smartTxPower);

void toa_state(toa_core_t *toa, toa_state_t state) { toa->state = state; }

// add new measure to measures table
void toa_add_measure(dev_addr_t addr, int distance)
{
  int16_t cRSSI, cFPP, cSNR;
  TRANSCEIVER_ReadDiagnostic(&cRSSI, &cFPP, &cSNR);
  LOG_INF("measure id:%X dist:%d rssi:%d", addr, distance, cRSSI);
}

int toa_find_addr_index_in_resp(toa_core_t *toa, dev_addr_t addr)
{
  int ind = 0;
  while (ind < toa->anc_in_poll_cnt)
  {
    if (toa->addr_tab[ind] == addr)
    {
      return ind;
    }
    ++ind;
  }
  return TOA_MAX_DEV_IN_POLL;
}

// calculate TimeOfFlight in DecaWave TimeUnits or 0
int toa_calc_tof_dw_tu(toa_core_t *toa, int resp_ind)
{
  const uint32_t TsErr = 0;
  if (toa->TsPollTx == TsErr || toa->TsPollRx == TsErr ||
      toa->TsRespTx == TsErr || toa->TsRespRx[resp_ind] ||
      toa->TsFinTx == TsErr || toa->TsFinRx)
  {
    return 0;
  }
  float Ra, Rb, Da, Db, den, t;
  Ra = (float)((uint32_t)(toa->TsRespRx[resp_ind] - (uint32_t)toa->TsPollTx));
  Rb = (float)((uint32_t)(toa->TsFinRx - toa->TsRespTx));
  Da = (float)((uint32_t)(toa->TsFinTx - toa->TsRespRx[resp_ind]));
  Db = (float)((uint32_t)(toa->TsRespTx - toa->TsPollRx));
  den = (Ra + Rb + Da + Db);
  t = ((Ra * Rb - Da * Db) / den * DWT_TIME_UNITS);
  return t;
}

// calculate TimeOfFlight in seconds
float toa_calc_tof(toa_core_t *toa, int resp_ind)
{
  return (float)toa_calc_tof_dw_tu(toa, resp_ind) * DWT_TIME_UNITS;
}

// substract returned value from measured distance [cm]
int toa_get_range_bias(int cm)
{
  const int ch = settings.transceiver.dwt_config.chan;
  const int prf = settings.transceiver.dwt_config.prf;
  const bool smart_power = ch == DWT_BR_6M8;
  return _toa_get_range_bias(ch, cm, prf, smart_power);
}

int toa_tof_to_cm(float tof)
{
  int cm = tof * SPEED_OF_LIGHT * 100;
  int ch = settings.transceiver.dwt_config.chan;
  int prf = settings.transceiver.dwt_config.prf;
  int smart_power = settings.transceiver.dwt_config.dataRate == DWT_BR_6M8;
  cm -= _toa_get_range_bias(ch, cm, prf, smart_power);
  return cm;
}

int64_t toa_set_tx_time(int64_t dw_time, uint32_t delay_us)
{
  dw_time += delay_us * UUS_TO_DWT_TIME;
  dw_time &= 0x00FFFFFFFE00;           // trim value to tx timer resolution
  dwt_setdelayedtrxtime(dw_time >> 8); // convert to tx timer unit
  return (dw_time + settings.transceiver.ant_dly_tx) & MASK_40BIT;
}

// called from response cb.
int toa_enable_rx_before_fin(toa_core_t *toa, toa_settings_t *tset)
{
  int64_t rx_start = tset->resp_dly[toa->resp_ind] - tset->guard_time;
  if (toa->resp_ind == toa->anc_in_poll_cnt)
  {
    rx_start += tset->fin_dly;
  }
  rx_start = rx_start * DWT_TIME_UNITS + toa->TsPollRx;
  dwt_setdelayedtrxtime(rx_start & MASK_40BIT);
  dwt_setrxtimeout(2 * settings.mac.sync_dly.guard_time);
  return dwt_rxenable(DWT_START_RX_DELAYED);
}

void toa_write_40b_value(uint8_t *dst, int64_t val)
{
  for (int i = 0; i < 5 * 8; i += 8)
  {
    *dst = 0xFF & (val >> i);
    ++dst;
  }
}

int64_t toa_read_40b_value(const uint8_t *src)
{
  int64_t val = 0;
  src += 5;
  for (int i = 0; i < 5 * 8; i += 8)
  {
    --src;
    val = (val << 8) + *src;
  }
  return val;
}
