/*
 * imu.h
 *
 *  Created on: 20.04.2018
 *      Author: Administrator
 */

#ifndef UWB_IMU_H_
#define UWB_IMU_H_

#include "platform/port.h"
#include "transceiver.h"

#define 			IMU_ACCEL_WOM_THRESHOLD 0b00010000
#define				IMU_NO_MOTION_PERIOD	5000			// When the time (in milis) run out and no motion is detected, device will go to sleep
volatile uint8_t	imu_sleep_mode;

void ImuWomConfig(void);
void ImuMotionControl(void);
void ImuIRQHandler(void);
void ImuWatchdogRefresh(void);

#endif /* UWB_IMU_H_ */
