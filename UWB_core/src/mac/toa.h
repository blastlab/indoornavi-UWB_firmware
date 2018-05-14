#ifndef _TOA_H__
#define _TOA_H__

#include "../decadriver/deca_device_api.h" // DWT_TIME_UNITS
#include "../logs.h"
#include "../settings.h"
#include "../transceiver.h" // read diagnostic

// speed of light in air [m/s]
#define SPEED_OF_LIGHT 299702547

typedef enum {
  TOA_IDLE,
  TOA_POLL_WAIT_TO_SEND,
  TOA_POLL_SENT,
  TOA_RESP_REC,
  TOA_FIN_WAIT_TO_SEND,
  TOA_FIN_SENT,
  TOA_POLL_REC,
  TOA_RESP_WAIT_TO_SEND,
  TOA_RESP_SENT,
  TOA_FIN_REC,
} toa_state_t;

typedef struct
{
  toa_state_t state, prev_state;
  int64_t TsPollTx;
  uint32_t TsRespRx[TOA_MAX_DEV_IN_POLL];
  uint32_t TsFinTx;
  uint32_t TsPollRx;
  uint32_t TsRespTx;
  uint32_t TsFinRx;
  uint8_t resp_ind;        // 0..anc_in_poll_cnt-1
  uint8_t anc_in_poll_cnt; // 1..TOA_MAX_DEV_IN_POLL
  dev_addr_t addr_tab[TOA_MAX_DEV_IN_POLL];
  dev_addr_t initiator;
} toa_core_t;

// change state of toa instance
void TOA_State(toa_core_t *toa, toa_state_t state);

// add new measure to measures table
void TOA_AddMeasure(dev_addr_t addr, int distance);

// return index of a given address in a toa->addr_tab
int TOA_FindAddrIndexInResp(toa_core_t *toa, dev_addr_t addr);

// calculate TimeOfFlight in DecaWave TimeUnits or 0
int TOA_CalcTofDwTu(toa_core_t *toa, int resp_ind);

// calculate TimeOfFlight in seconds
float TOA_CalcTof(toa_core_t *toa, int resp_ind);

// convert tof to cm including bias correction
int TOA_TofToCm(float tof);

// set delayed transmission parameters and return future TX timestamp
int64_t TOA_SetTxTime(int64_t dw_time, uint32_t delay_us);

// called from response cb.
int TOA_EnableRxBeforeFin(toa_core_t *toa, toa_settings_t *tset, uint64_t DwPollRxTs);

// usefull when writing full timestamp
void TOA_Write40bValue(uint8_t *dst, int64_t val);

// usefull when reading full timestamp
int64_t TOA_Read40bValue(const uint8_t *src);
#endif
