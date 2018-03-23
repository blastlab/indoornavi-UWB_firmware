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
  toa_state_t state;
  int64_t TsPollTx;
  uint32_t TsPollRx;
  uint32_t TsRespTx, TsRespRx[TOA_MAX_DEV_IN_POLL];
  uint32_t TsFinTx, TsFinRx;
  uint8_t resp_ind;        // 0..anc_in_poll_cnt-1
  uint8_t anc_in_poll_cnt; // 1..TOA_MAX_DEV_IN_POLL
  dev_addr_t addr_tab[TOA_MAX_DEV_IN_POLL];
  dev_addr_t initiator;
} toa_core_t;

// change state of toa instance
void toa_state(toa_core_t *toa, toa_state_t state);

// add new measure to measures table
void toa_add_measure(dev_addr_t addr, int distance);

// return index of a given address in a toa->addr_tab
int toa_find_addr_index_in_resp(toa_core_t *toa, dev_addr_t addr);

// calculate TimeOfFlight in DecaWave TimeUnits or 0
int toa_calc_tof_dw_tu(toa_core_t *toa, int resp_ind);

// calculate TimeOfFlight in seconds
float toa_calc_tof(toa_core_t *toa, int resp_ind);

// convert tof to cm including bias correction
int toa_tof_to_cm(float tof);

// set delayed transmission parameters and return future TX timestamp
int64_t toa_set_tx_time(int64_t dw_time, uint32_t delay_us);

// called from response cb.
int toa_enable_rx_before_fin(toa_core_t *toa, toa_settings_t *tset);

// usefull when writing full timestamp
void toa_write_40b_value(uint8_t *dst, int64_t val);

// usefull when reading full timestamp
int64_t toa_read_40b_value(const uint8_t *src);
#endif
