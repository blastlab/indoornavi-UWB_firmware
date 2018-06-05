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

#define 			IMU_ACCEL_WOM_THRESHOLD 0b01000000
#define				IMU_NO_MOTION_PERIOD	5000			// When the time (in milis) run out and no motion is detected, device will go to sleep
volatile uint8_t	imu_sleep_mode;
uint16_t 			fifo_count;

enum { A_DLPF_CFG = 7, DEC2_CFG = 3, ACCEL_FS_SEL = 1 };					// accel config; A_DLPF_CFG[0-7]; DEC2_CFG[0-3]; ACCEL_FS_SEL[0-3];		41st page
enum { G_DLPF_CFG = 7, G_AVGCFG = 5, FS_SEL = 3 }; 							// gyro config;  G_DLPF_CFG[0-7]; G_AVGCFG[0-7]; FS_SEL[0-3];			42nd page

#define				GYRO_DIV 				(16.384*pow(2.0, 3.0 - FS_SEL));
#define 			ACC_DIV 				(pow(2.0, 14.0 - ACCEL_FS_SEL)/pow(2.0, DEC2_CFG));
#define 			SAMPLE_RATE_DIV 		19

#define 			SAMPLE_F				(double)(1000.0/(1 + SAMPLE_RATE_DIV))			// 100Hz sampling
#define 			SAMPLE_COUNT			36
#define 			DEG_TO_RAD				3.14159265358979f/180.0f

#define 			deltat 					(float)(1.0f/(float)SAMPLE_F)					// sampling period in seconds
#define 			gyroMeasError 			3.14159265358979f * (5.0f / 180.0f) 			// gyroscope measurement error in rad/s (shown as 5 deg/s)
#define 			beta 					sqrt(3.0f / 4.0f) * gyroMeasError 				// compute beta

float buff_a_x, buff_a_y, buff_a_z, buff_a_a;
float r_acc_X, r_acc_Y, r_acc_Z;
float buff_w_x, buff_w_y, buff_w_z;
float SEq_1, SEq_2, SEq_3, SEq_4;
float yaw, pitch, roll;

float v_x, v_y, v_z;
float pos_x, pos_y, pos_z;

void ImuFifoConfig(void);
void ImuWomConfig(void);
void ImuMotionControl(void);
void ImuIRQHandler(void);
void ImuWatchdogRefresh(void);

#endif /* UWB_IMU_H_ */
