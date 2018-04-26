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
#include <math.h>

#define 			IMU_ACCEL_WOM_THRESHOLD 0b00010000
#define				IMU_NO_MOTION_PERIOD	5000			// When the time (in milis) run out and no motion is detected, device will go to sleep
volatile uint8_t	imu_sleep_mode;

uint16_t 			fifo_count;
#define 			SAMPLE_RATE_DIV 		0x9
#define 			SAMPLE_F				(double)(1000.0/(1 + SAMPLE_RATE_DIV))			// 100Hz sampling
#define 			SAMPLE_COUNT			120
#define 			DEG_TO_RAD				3.14159265358979f/180.0f

#define 			deltat 					(float)(1.0f/(float)SAMPLE_F)					// sampling period in seconds
#define 			gyroMeasError 			3.14159265358979f * (5.0f / 180.0f) 			// gyroscope measurement error in rad/s (shown as 5 deg/s)
#define 			beta 					sqrt(3.0f / 4.0f) * gyroMeasError 				// compute beta

float a_x, a_y, a_z, a_a;
float w_x, w_y, w_z;
float SEq_1, SEq_2, SEq_3, SEq_4;
float yaw, pitch, roll;

void ImuFifoConfig(void);
void ImuWomConfig(void);
void ImuMotionControl(void);
void ImuIRQHandler(void);
void ImuWatchdogRefresh(void);

#endif /* UWB_IMU_H_ */
