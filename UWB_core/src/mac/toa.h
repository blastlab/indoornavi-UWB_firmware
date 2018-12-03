#ifndef _TOA_H__
#define _TOA_H__

#include "../decadriver/deca_device_api.h"  // DWT_TIME_UNITS
#include "../logger/logs.h"
#include "../settings.h"
#include "../transceiver.h"  // read diagnostic

/// speed of light in air [m/s]
#define SPEED_OF_LIGHT 299702547

#define TOA_ASSERT IASSERT

/// set 1 to trace sync debug messages
#define TOA_TRACE_ENABLED 0
#if TOA_TRACE_ENABLED
#define TOA_TRACE(...) LOG_DBG(__VA_ARGS__)
#else
#include "tools.h"
#define TOA_TRACE(...) ALL_UNUSED(__VA_ARGS__)
#endif

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

typedef struct {
	toa_state_t state, prev_state;
	int64_t TsPollRx, TsPollTx;
	uint32_t TsRespRx[TOA_MAX_DEV_IN_POLL];
	uint32_t TsFinTx;
	uint32_t TsRespTx;
	uint32_t TsFinRx;
	uint8_t resp_ind;                          ///< 0..anc_in_poll_cnt-1
	uint8_t anc_in_poll_cnt;                   ///< 1..TOA_MAX_DEV_IN_POLL
	dev_addr_t addr_tab[TOA_MAX_DEV_IN_POLL];  ///< anchors addresses
	dev_addr_t initiator;                      ///< address of measure initiator
} toa_core_t;

typedef struct {
	uint16_t did1, did2;
	int16_t dist_cm;    ///< distance in cm
	int16_t rssi_cdbm;  ///< rssi in centy db
	int16_t fpp_cdbm;   ///< first path power in centy db
	int16_t snr_cdbm;   ///< signal to noise ratio in centy dbm
}__packed measure_t;

typedef struct {
	toa_core_t core;
	measure_t measures[TOA_MEASURES_BUF_SIZE];
	int measures_read_ind;
	int measures_write_ind;
} toa_instance_t;

/**
 * @brief change state of toa instance
 *
 * @param toa data
 * @param state new state
 */
void TOA_State(toa_core_t* toa, toa_state_t state);

/**
 * @brief read measure info from transceiver and add data to measure table
 *
 * readed RSSI, FPP and SNR is in centy dB value to avoid sending float
 *
 * @param[in] addr of remote device
 * @param[in] distance in cm
 */
void TOA_MeasurePushLocal(dev_addr_t addr, int distance);

/**
 * @brief add new measure to measures table
 *
 * @param meas measure informations
 */
void TOA_MeasurePush(const measure_t* meas);

/**
 * @brief return pointer to the oldest measure
 *
 * @return measure_t* pointer to the oldest measure or null
 */
const measure_t* TOA_MeasurePeek();

/**
 * @brief return oldest measure and release slot in table
 *
 * @return measure_t* pointer to the oldest measure or null
 */
measure_t* TOA_MeasurePop();

/**
 * @brief return index of a given address in a toa->addr_tab
 *
 * @param[in] toa data
 * @param[in] addr address to find
 * @return int index of device of TOA_MAX_DEV_IN_POLL
 */
int TOA_FindAddrIndexInResp(toa_core_t* toa, dev_addr_t addr);

/**
 * @brief calculate TimeOfFlight in DecaWave TimeUnits or 0
 *
 * @param[in] toa data
 * @param[in] resp_ind index of response
 * @return int tof in dtu
 */
int TOA_CalcTofDwTu(const toa_core_t* toa, int resp_ind);

/**
 * @brief calculate TimeOfFlight in seconds
 *
 * @param[in] toa data
 * @param[in] resp_ind index of response
 * @return float tof in seconds
 */
float TOA_CalcTof(const toa_core_t* toa, int resp_ind);

/**
 * @brief convert tof to cm including bias correction
 *
 * @param[in] tof in seconds
 * @return int distance in cm
 */
int TOA_TofToCm(float tof);

/**
 * @brief set delayed transmission parameters and return future TX timestamp
 *
 * @param[in] dw_time start local time in dru
 * @param[in] delay_us time delay in micro seconds
 * @return int64_t transmission time in dtu
 */
int64_t TOA_SetTxTime(int64_t dw_time, uint32_t delay_us);

/**
 * @brief enable receiver just before fin message
 *
 * @param[in] toa pointer to toa data
 * @param[in] tset pointer to toa settings
 * @param DwPollRxTs time in local dtu
 * @return int 0 if sussess or error code
 */
int TOA_EnableRxBeforeFin(const toa_core_t* toa, const toa_settings_t* tset, uint64_t DwPollRxTs);

/**
 * @brief usefull when writing full timestamp
 *
 * @param[out] dst pointer to destination address
 * @param val value to write
 */
void TOA_Write40bValue(uint8_t* dst, int64_t val);

/**
 * @brief usefull when reading full timestamp
 *
 * @param[in] src pointer to address in LittleEndian
 * @return int64_t 40b address
 */
int64_t TOA_Read40bValue(const uint8_t* src);
#endif
