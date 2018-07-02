#include "toa.h"

// private function from calib file
int _TOA_GetRangeBias(uint8 chan, int range, uint8 prf, int smartTxPower);

void TOA_State(toa_core_t *toa, toa_state_t state) {
	toa->prev_state = toa->state;
	toa->state = state;
}

// add new measure to measures table
void TOA_AddMeasure(dev_addr_t addr, int distance) {
  int16_t cRSSI, cFPP, cSNR;
  TRANSCEIVER_ReadDiagnostic(&cRSSI, &cFPP, &cSNR);
  LOG_INF("measure id:%X dist:%d rssi:%d", addr, distance, cRSSI);
}

int TOA_FindAddrIndexInResp(toa_core_t *toa, dev_addr_t addr) {
  int ind = 0;
  while (ind < toa->anc_in_poll_cnt) {
    if (toa->addr_tab[ind] == addr) {
      return ind;
    }
    ++ind;
  }
  return TOA_MAX_DEV_IN_POLL;
}

// calculate TimeOfFlight in DecaWave TimeUnits or 0
int TOA_CalcTofDwTu(const toa_core_t *toa, int resp_ind) {
  const uint32_t TsErr = 0;
  if (toa->TsPollTx == TsErr || toa->TsPollRx == TsErr ||
      toa->TsRespTx == TsErr || toa->TsRespRx[resp_ind] == TsErr ||
      toa->TsFinTx == TsErr || toa->TsFinRx == TsErr) {
    return 0;
  }
  float Ra, Rb, Da, Db, den, t;
  Ra = (float)((uint32_t)(toa->TsRespRx[resp_ind] - (uint32_t)toa->TsPollTx));
  Rb = (float)((uint32_t)(toa->TsFinRx - toa->TsRespTx));
  Da = (float)((uint32_t)(toa->TsFinTx - toa->TsRespRx[resp_ind]));
  Db = (float)((uint32_t)(toa->TsRespTx - toa->TsPollRx));
  den = (Ra + Rb + Da + Db);
  t = ((Ra * Rb - Da * Db) / den);
  return t;
}

// calculate TimeOfFlight in seconds
float TOA_CalcTofconst (toa_core_t *toa, int resp_ind) {
  return (float)TOA_CalcTofDwTu(toa, resp_ind) * DWT_TIME_UNITS;
}

// substract returned value from measured distance [cm]
int TOA_GetRangeBias(int cm) {
  const int ch = settings.transceiver.dwt_config.chan;
  const int prf = settings.transceiver.dwt_config.prf;
  const bool smart_power = ch == DWT_BR_6M8;
  return _TOA_GetRangeBias(ch, cm, prf, smart_power);
}

int TOA_TofToCm(float tof) {
  int cm = tof * SPEED_OF_LIGHT * 100;
  int ch = settings.transceiver.dwt_config.chan;
  int prf = settings.transceiver.dwt_config.prf;
  int smart_power = settings.transceiver.dwt_config.dataRate == DWT_BR_6M8;
  cm -= _TOA_GetRangeBias(ch, cm, prf, smart_power);
  return cm;
}

int64_t TOA_SetTxTime(int64_t dw_time, uint32_t delay_us) {
  dw_time += delay_us * UUS_TO_DWT_TIME;
  dw_time &= 0x00FFFFFFFE00;           // trim value to tx timer resolution
  dwt_setdelayedtrxtime(dw_time >> 8); // convert to tx timer unit
  return (dw_time + settings.transceiver.ant_dly_tx) & MASK_40BIT;
}

// called from response cb.
int TOA_EnableRxBeforeFin(const toa_core_t *toa, const toa_settings_t *tset,
                          uint64_t DwPollRxTs) {
  int64_t rx_start = tset->resp_dly_us[toa->resp_ind] - tset->guard_time_us;
  if (toa->resp_ind == toa->anc_in_poll_cnt) {
    rx_start += tset->fin_dly_us;
  }
  rx_start = rx_start * DWT_TIME_UNITS + DwPollRxTs;
  dwt_setdelayedtrxtime(rx_start & MASK_40BIT);
  dwt_setrxtimeout(2 * settings.mac.sync_dly.guard_time_us);
  return dwt_rxenable(DWT_START_RX_DELAYED);
}

void TOA_Write40bValue(uint8_t *dst, int64_t val) {
  for (int i = 0; i < 5 * 8; i += 8) {
    *dst = 0xFF & (val >> i);
    ++dst;
  }
}

int64_t TOA_Read40bValue(const uint8_t *src) {
  int64_t val = 0;
  src += 5;
  for (int i = 0; i < 5 * 8; i += 8) {
    --src;
    val = (val << 8) + *src;
  }
  return val;
}
