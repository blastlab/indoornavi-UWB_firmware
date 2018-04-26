/*
 * imu.c
 *
 *  Created on: 20.04.2018
 *      Author: Dawid Peplinski
 */
#include "imu.h"

void ImuSetGyroOffset(void)
{
	uint8_t data[6] = { 0 };
	PORT_ImuWriteRegister(0x13, 0);
	PORT_ImuWriteRegister(0x14, 0);
	PORT_ImuWriteRegister(0x15, 0);
	PORT_ImuWriteRegister(0x16, 0);
	PORT_ImuWriteRegister(0x17, 0);
	PORT_ImuWriteRegister(0x18, 0);

	PORT_SleepMs(50);

	PORT_ImuReadRegister(0x43, &data[0], 6);	// reading GYRO's values, setting the offset
	int16_t offs = -2*(int16_t)(data[0] << 8 | data[1]);
	PORT_ImuWriteRegister(0x13, (int8_t)(offs >> 8));
	PORT_ImuWriteRegister(0x14, (int8_t)offs);
	offs = -2*(int16_t)(data[2] << 8 | data[3]);
	PORT_ImuWriteRegister(0x15, (int8_t)(offs >> 8));
	PORT_ImuWriteRegister(0x16, (int8_t)offs);
	offs = -2*(int16_t)(data[4] << 8 | data[5]);
	PORT_ImuWriteRegister(0x17, (int8_t)(offs >> 8));
	PORT_ImuWriteRegister(0x18, (int8_t)offs);
}

uint16_t ImuGetFifoCount(void)
{
	uint8_t dat[2] = { 0 };
	PORT_ImuReadRegister(0x72, &dat[0], 1);
	PORT_ImuReadRegister(0x73, &dat[1], 1);
	return ((0b00011111 & dat[0]) << 8) | dat[1];
}

void filterUpdate(float w_x_t, float w_y_t, float w_z_t, float a_x_t, float a_y_t, float a_z_t)
{
	// Local system variables
	float norm; 															// vector norm
	float SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4; 	// quaternion derrivative from gyroscopes elements
	float f_1, f_2, f_3; 													// objective function elements
	float J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33; 				// objective function Jacobian elements
	float SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4; 				// estimated direction of the gyroscope error

	// Axulirary variables to avoid reapeated calcualtions
	float halfSEq_1 = 0.5f * SEq_1;
	float halfSEq_2 = 0.5f * SEq_2;
	float halfSEq_3 = 0.5f * SEq_3;
	float halfSEq_4 = 0.5f * SEq_4;
	float twoSEq_1 = 2.0f * SEq_1;
	float twoSEq_2 = 2.0f * SEq_2;
	float twoSEq_3 = 2.0f * SEq_3;

	// Normalise the accelerometer measurement
	norm = sqrt(a_x_t * a_x_t + a_y_t * a_y_t + a_z_t * a_z_t);
	a_x_t /= norm;
	a_y_t /= norm;
	a_z_t /= norm;

	// Compute the objective function and Jacobian
	f_1 = twoSEq_2 * SEq_4 - twoSEq_1 * SEq_3 - a_x_t;
	f_2 = twoSEq_1 * SEq_2 + twoSEq_3 * SEq_4 - a_y_t;
	f_3 = 1.0f - twoSEq_2 * SEq_2 - twoSEq_3 * SEq_3 - a_z_t;
	J_11or24 = twoSEq_3; 													// J_11 negated in matrix multiplication
	J_12or23 = 2.0f * SEq_4;
	J_13or22 = twoSEq_1; 													// J_12 negated in matrix multiplication
	J_14or21 = twoSEq_2;
	J_32 = 2.0f * J_14or21; 												// negated in matrix multiplication
	J_33 = 2.0f * J_11or24; 												// negated in matrix multiplication

	// Compute the gradient (matrix multiplication)
	SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1;
	SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3;
	SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1;
	SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2;

	// Normalise the gradient
	norm = sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
	SEqHatDot_1 /= norm;
	SEqHatDot_2 /= norm;
	SEqHatDot_3 /= norm;
	SEqHatDot_4 /= norm;

	// Compute the quaternion derrivative measured by gyroscopes
	SEqDot_omega_1 = -halfSEq_2 * w_x_t - halfSEq_3 * w_y_t - halfSEq_4 * w_z_t;
	SEqDot_omega_2 = halfSEq_1 * w_x_t + halfSEq_3 * w_z_t - halfSEq_4 * w_y_t;
	SEqDot_omega_3 = halfSEq_1 * w_y_t - halfSEq_2 * w_z_t + halfSEq_4 * w_x_t;
	SEqDot_omega_4 = halfSEq_1 * w_z_t + halfSEq_2 * w_y_t - halfSEq_3 * w_x_t;

	// Compute then integrate the estimated quaternion derrivative
	SEq_1 += (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
	SEq_2 += (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
	SEq_3 += (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
	SEq_4 += (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;

	// Normalise quaternion
	norm = sqrt(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
	SEq_1 /= norm;
	SEq_2 /= norm;
	SEq_3 /= norm;
	SEq_4 /= norm;
}

void ImuFifoConfig(void)
{
	yaw = 0.0f;
	pitch = 0.0f;
	roll = 0.0f;
	SEq_1 = 0.0f;
	SEq_2 = 0.0f;
	SEq_3 = 0.0f;
	SEq_4 = 1.0f;
	PORT_ImuReset();
	PORT_ImuWriteRegister(0x6b, 0b00001001);		// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_ImuWriteRegister(0x6c, 0b11000000);		// PWR_MGMT_2 register; turning on accelerometer and gyro, turning on FIFO low power mode
	PORT_ImuWriteRegister(0x19, SAMPLE_RATE_DIV);	// SMPLRT_DIV register; dividing the sampling rate (internal sample rate = 1kHz)
	PORT_ImuWriteRegister(0x1A, 0b01000101);		// CONFIG register;
	PORT_ImuWriteRegister(0x1b, 0b00011000);		// GYRO_CONFIG register; -/+ 2000 dps
	PORT_ImuWriteRegister(0x1c, 0b00000000); 		// ACCEL_CONFIG register; +/- 2G full scale select
	PORT_ImuWriteRegister(0x1d, 0b11010000); 		// ACCEL_CONFIG2 register;
	PORT_ImuWriteRegister(0x1e, 0b10010000);		// LP_MODE_CFG register;
	PORT_ImuWriteRegister(0x23, 0b01111000);		// FIFO_EN register;
	PORT_ImuWriteRegister(0x37, 0b00000000);		// INT_PIN_CFG register;
	HAL_Delay(1000);
	ImuSetGyroOffset();

	PORT_ImuWriteRegister(0x38, 0b00010000);		// INT_ENABLE register; enabling FIFO overflow interrupt on accelerometer
	PORT_ImuWriteRegister(0x6a, 0b01010100);		// USER_CTRL register; enabling and resetting FIFO

	uint8_t fifo_data[SAMPLE_COUNT] = { 0 };
	while(1)
	{
		fifo_count = ImuGetFifoCount();
		if(fifo_count > SAMPLE_COUNT)
		{
			PORT_ImuReadRegister(0x74, &fifo_data[0], SAMPLE_COUNT);
			for(uint16_t i = 0; i < SAMPLE_COUNT; i += 12)
			{
				a_x = (float)((int16_t)((fifo_data[i] << 8) | fifo_data[i + 1]))/8192.0f;
				a_y = (float)((int16_t)((fifo_data[i + 2] << 8) | fifo_data[i + 3]))/8192.0f;
				a_z = (float)((int16_t)((fifo_data[i + 4] << 8) | fifo_data[i + 5]))/8192.0f;
				a_a = sqrt(a_x * a_x + a_y * a_y + a_z * a_z);
				w_x = (float)((int16_t)((fifo_data[i + 6] << 8) | fifo_data[i + 7]))/16.384f;
				w_y = (float)((int16_t)((fifo_data[i + 8] << 8) | fifo_data[i + 9]))/16.384f;
				w_z = (float)((int16_t)((fifo_data[i + 10] << 8) | fifo_data[i + 11]))/16.384f;
				filterUpdate(w_x*DEG_TO_RAD, w_y*DEG_TO_RAD, w_z*DEG_TO_RAD, a_x, a_y, a_z);
			}
			yaw = atan2(2.0*(SEq_2*SEq_3 + SEq_4*SEq_1), SEq_4*SEq_4 - SEq_1*SEq_1 - SEq_2*SEq_2 + SEq_3*SEq_3);		// euler's coords
			pitch = asin(-2.0*(SEq_1*SEq_3 - SEq_4*SEq_2));
			roll = atan2(2.0*(SEq_1*SEq_2 + SEq_4*SEq_3), SEq_4*SEq_4 + SEq_1*SEq_1 - SEq_2*SEq_2 - SEq_3*SEq_3);
			fifo_count = ImuGetFifoCount();
		}
	}
//	while(1)		// integration with trapezoids
//	{
//		fifo_count = imu_get_fifo_count();
//		if(fifo_count > SAMPLE_COUNT)
//		{
//			PORT_ImuReadRegister(0x74, &fifo_data[0], SAMPLE_COUNT);
//			for(uint16_t i = 6; i < (SAMPLE_COUNT - 12); i += 12)
//			{
//				w_x += (double)((int16_t)((fifo_data[i] << 8) | fifo_data[i + 1]) + (int16_t)((fifo_data[i + 12] << 8) | fifo_data[i + 13]))/SAMPLE_F/2.0;
//				w_y += (double)((int16_t)((fifo_data[i + 2] << 8) | fifo_data[i + 3]) + (int16_t)((fifo_data[i + 14] << 8) | fifo_data[i + 15]))/SAMPLE_F/2.0;
//				w_z += (double)((int16_t)((fifo_data[i + 4] << 8) | fifo_data[i + 5]) + (int16_t)((fifo_data[i + 16] << 8) | fifo_data[i + 17]))/SAMPLE_F/2.0;
//			}
//			fifo_count = imu_get_fifo_count();
//		}
//	}
}

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


void ImuReadAllConfig(void)
{
	PORT_ImuReset();
	PORT_ImuWriteRegister(0x6b, 0b00001001);	// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_ImuWriteRegister(0x6a, 0b00010000);	// USER_CTRL register; disabling I2C
	PORT_ImuWriteRegister(0x1a, 0b00000000);	// CONFIG register;
	PORT_ImuWriteRegister(0x1b, 0b00011000);	// GYRO_CONFIG register; -/+ 2000 dps
	PORT_ImuWriteRegister(0x1c, 0b00000000); 	// ACCEL_CONFIG register;
	PORT_ImuWriteRegister(0x1d, 0b00000000); 	// ACCEL_CONFIG2 register;
	ImuSetGyroOffset();
}
