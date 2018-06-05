/*
 * imu.c
 *
 *  Created on: 20.04.2018
 *      Author: Dawid Peplinski
 */
#include "imu.h"

static volatile uint32_t motion_tick;

void ImuWomConfig(void) {
	motion_tick = 0;
	imu_sleep_mode = 0;
	PORT_ImuReset();
	PORT_ImuWriteRegister(0x6b, 0b00001001);				// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_ImuWriteRegister(0x6c, 0b00000111);				// PWR_MGMT_2 register; turning on accelerometer and turning off gyro
	PORT_ImuWriteRegister(0x1c, 0b00000000); 				// ACCEL_CONFIG; +/- 2G full scale select
	PORT_ImuWriteRegister(0x1d, 0b00000001);				// ACCEL_CONFIG2 register; clearing ACCEL_FCHOICE_B, setting A_DLPF_CFG to 1
	PORT_ImuWriteRegister(0x38, 0b11100000);				// INT_ENABLE register; enabling WoM interrupt on accelerometer
	PORT_ImuWriteRegister(0x37, 0b00000000);				// INT_PIN_CFG register;
	PORT_ImuWriteRegister(0x20, IMU_ACCEL_WOM_THRESHOLD);	// ACCEL_WOM_X_THR register; threshold value for WoM
	PORT_ImuWriteRegister(0x21, IMU_ACCEL_WOM_THRESHOLD);	// ACCEL_WOM_Y_THR register;
	PORT_ImuWriteRegister(0x22, IMU_ACCEL_WOM_THRESHOLD);	// ACCEL_WOM_Z_THR register;
	PORT_ImuWriteRegister(0x69, 0b11000000);				// ACCEL_INTEL_CTRL register; setting ACCEL_INTEL_EN and ACCEL_INTEL_MODE (Wake-on-Motion detection logic)
	PORT_ImuWriteRegister(0x19, 0b10000000);				// SMPLRT_DIV register; dividing the sampling rate (internal sample rate = 1kHz)
	PORT_ImuWriteRegister(0x6b, 0b00101001);				// PWR_MGMT_1 register; enabling low-power cycle mode for accelerometer
}

void ImuMotionControl(void) {
	if((PORT_TickMs() - motion_tick) > IMU_NO_MOTION_PERIOD) {
		imu_sleep_mode = 1;
		TRANSCEIVER_EnterSleep();
		PORT_PrepareSleepMode();
		while(imu_sleep_mode) {
			PORT_EnterSleepMode();
		}
		PORT_ExitSleepMode();
		uint8_t *dummy_buf = malloc(256);
		TRANSCEIVER_WakeUp(dummy_buf, 256);
		TRANSCEIVER_DefaultRx();
		free(dummy_buf);
	}
}

inline void ImuIRQHandler(void) {
	motion_tick = PORT_TickMs();
	imu_sleep_mode = 0;
}

void ImuWatchdogRefresh(void) {
	if(imu_sleep_mode) PORT_WatchdogRefresh();
}
