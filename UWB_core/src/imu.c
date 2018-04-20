/*
 * imu.c
 *
 *  Created on: 20.04.2018
 *      Author: Dawid Peplinski
 */
#include "imu.h"

void PORT_imuSetGyroOffset(void)
{
	uint8_t data[6] = { 0 };
	PORT_ImuReadRegister(0x43, &data[0], 6);	// reading GYRO's values, setting the offset
	int8_t offs_offs = 1;
	int16_t offs = -2*(data[0] << 8 | data[1]) - offs_offs; // -17;
	PORT_ImuWriteRegister(0x13, (int8_t)(offs >> 8));
	PORT_ImuWriteRegister(0x14, (int8_t)offs);
	offs = -2*(data[2] << 8 | data[3]) - offs_offs; // -107;
	PORT_ImuWriteRegister(0x15, (int8_t)(offs >> 8));
	PORT_ImuWriteRegister(0x16, (int8_t)offs);
	offs = -2*(data[4] << 8 | data[5]) - offs_offs; // 6;
	PORT_ImuWriteRegister(0x17, (int8_t)(offs >> 8));
	PORT_ImuWriteRegister(0x18, (int8_t)offs);
}

void PORT_imuReadAllConfig(void)
{
	PORT_ImuReset();
	PORT_ImuWriteRegister(0x6b, 0b00001001);	// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_ImuWriteRegister(0x6a, 0b00010000);	// USER_CTRL register; disabling I2C
	PORT_ImuWriteRegister(0x1a, 0b00000000);	// CONFIG register;
	PORT_ImuWriteRegister(0x1b, 0b00011000);	// GYRO_CONFIG register; -/+ 2000 dps
	PORT_ImuWriteRegister(0x1c, 0b00000000); 	// ACCEL_CONFIG register;
	PORT_ImuWriteRegister(0x1d, 0b00000000); 	// ACCEL_CONFIG2 register;
	PORT_imuSetGyroOffset();
}

double x_degrees, y_degrees, z_degrees;
uint16_t fifo_count = 0;
#define SAMPLE_RATE_DIV 	0b00000010
#define SAMPLE_F			(double)(1000.0/(1 + SAMPLE_RATE_DIV))

void imuFifoConfig(void)
{
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
	HAL_Delay(100);
	PORT_imuSetGyroOffset();

	PORT_ImuWriteRegister(0x38, 0b00010000);		// INT_ENABLE register; enabling FIFO overflow interrupt on accelerometer
	PORT_ImuWriteRegister(0x6a, 0b01010100);		// USER_CTRL register; enabling and resetting FIFO

	x_degrees = 0;
	y_degrees = 0;
	z_degrees = 0;
	uint8_t dat[2] = { 0 };
	uint8_t fifo_data[1440] = { 0 };

	while(1)
	{
		PORT_ImuReadRegister(0x72, &dat[0], 1);
		PORT_ImuReadRegister(0x73, &dat[1], 1);
		fifo_count = ((0b00011111 & dat[0]) << 8) | dat[1];
		if(fifo_count > 1440)
		{
			PORT_ImuReadRegister(0x74, &fifo_data[0], 1440);
			for(uint16_t i = 6; i < (1440 - 12); i+=12)
			{
				x_degrees += (double)((int16_t)((fifo_data[i] << 8) | fifo_data[i + 1]) + (int16_t)((fifo_data[i + 12] << 8) | fifo_data[i + 13]))/SAMPLE_F/2.0;
				y_degrees += (double)((int16_t)((fifo_data[i + 2] << 8) | fifo_data[i + 3]) + (int16_t)((fifo_data[i + 14] << 8) | fifo_data[i + 15]))/SAMPLE_F/2.0;
				z_degrees += (double)((int16_t)((fifo_data[i + 4] << 8) | fifo_data[i + 5]) + (int16_t)((fifo_data[i + 16] << 8) | fifo_data[i + 17]))/SAMPLE_F/2.0;
			}
			PORT_ImuReadRegister(0x72, &dat[0], 1);
			PORT_ImuReadRegister(0x73, &dat[1], 1);
			fifo_count = ((0b00011111 & dat[0]) << 8) | dat[1];
		}
	}
}

static volatile uint32_t motion_tick;

void ImuWomConfig(void) {
	motion_tick = 0;
	imu_sleep_mode = 0;
	PORT_ImuReset();
	PORT_ImuWriteRegister(0x6b, 0b00001001);			// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_ImuWriteRegister(0x6c, 0b00000111);			// PWR_MGMT_2 register; turning on accelerometer and turning off gyro
	PORT_ImuWriteRegister(0x1c, 0b00000000); 			// ACCEL_CONFIG; +/- 2G full scale select
	PORT_ImuWriteRegister(0x1d, 0b00000001);			// ACCEL_CONFIG2 register; clearing ACCEL_FCHOICE_B, setting A_DLPF_CFG to 1
	PORT_ImuWriteRegister(0x38, 0b11100000);			// INT_ENABLE register; enabling WoM interrupt on accelerometer
	PORT_ImuWriteRegister(0x37, 0b00000000);			// INT_PIN_CFG register;
	PORT_ImuWriteRegister(0x20, IMU_ACCEL_WOM_THRESHOLD);	// ACCEL_WOM_X_THR register; threshold value for WoM
	PORT_ImuWriteRegister(0x21, IMU_ACCEL_WOM_THRESHOLD);	// ACCEL_WOM_Y_THR register;
	PORT_ImuWriteRegister(0x22, IMU_ACCEL_WOM_THRESHOLD);	// ACCEL_WOM_Z_THR register;
	PORT_ImuWriteRegister(0x69, 0b11000000);			// ACCEL_INTEL_CTRL register; setting ACCEL_INTEL_EN and ACCEL_INTEL_MODE (Wake-on-Motion detection logic)
	PORT_ImuWriteRegister(0x19, 0b10000000);			// SMPLRT_DIV register; dividing the sampling rate (internal sample rate = 1kHz)
	PORT_ImuWriteRegister(0x6b, 0b00101001);			// PWR_MGMT_1 register; enabling low-power cycle mode for accelerometer
}

void ImuMotionControl(void) {
	if((HAL_GetTick() - motion_tick) > IMU_NO_MOTION_PERIOD) {
		imu_sleep_mode = 1;
	}
}

inline void ImuIRQHandler(void) {
	motion_tick = HAL_GetTick();
	imu_sleep_mode = 0;
}
