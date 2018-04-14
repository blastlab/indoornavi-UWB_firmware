/*
 * port_imu.c
 *
 *  Created on: 14.04.2018
 *      Author: Dawid Peplinski
 */

#include "port.h"

extern SPI_HandleTypeDef hspi3;

void PORT_imuWriteRegister(uint8_t addr, uint8_t val)
{
	uint8_t data[] = { addr, val };
	while(HAL_SPI_GetState(&hspi3) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, &data[0], sizeof(data), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

void PORT_imuReadRegister(uint8_t addr, uint8_t *val, uint16_t count)
{
	uint8_t m_addr = addr | 0b10000000;
	while(HAL_SPI_GetState(&hspi3) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, &m_addr, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi3, val, count, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

void PORT_imuReset(void)
{
	PORT_imuWriteRegister(0x6b, 0b10000000);	// PWR_MGMT_1 register; reset
	HAL_Delay(100);
	PORT_imuWriteRegister(0x6b, 0b00000000);	// PWR_MGMT_1 register; unreset
	uint8_t data = 0;
	PORT_imuReadRegister(0x6b, &data, 1);
	if (data & (1<<7))						// if the device keeps reset state
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}

void PORT_imuSetGyroOffset(void)
{
	uint8_t data[6] = { 0 };
	PORT_imuReadRegister(0x43, &data[0], 6);	// reading GYRO's values, setting the offset
	for(uint8_t i = 0; i < 6; i += 2)
	{
	  PORT_imuWriteRegister(0x13 + i, (int8_t)((-1*(int16_t)((data[i] << 8) | data[i + 1])) >> 8));
	  PORT_imuWriteRegister(0x14 + i, (int8_t)((-1*(int16_t)((data[i] << 8) | data[i + 1])) & 0xFF));
	}
}

void PORT_imuReadAllConfig(void)
{
	PORT_imuReset();
	PORT_imuWriteRegister(0x6b, 0b00001001);	// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_imuWriteRegister(0x6a, 0b00010000);	// USER_CTRL register; disabling I2C
	PORT_imuWriteRegister(0x1a, 0b00000000);	// CONFIG register;
	PORT_imuWriteRegister(0x1b, 0b00011000);	// GYRO_CONFIG register; -/+ 2000 dps
	PORT_imuWriteRegister(0x1c, 0b00000000); 	// ACCEL_CONFIG register;
	PORT_imuWriteRegister(0x1d, 0b00000000); 	// ACCEL_CONFIG2 register;
	PORT_imuSetGyroOffset();
}

void PORT_imuWomConfig(void)
{
	PORT_imuReset();
	PORT_imuWriteRegister(0x6b, 0b00001001);	// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_imuWriteRegister(0x6c, 0b00000111);	// PWR_MGMT_2 register; turning on accelerometer and turning off gyro
	PORT_imuWriteRegister(0x1c, 0b00000000); 	// ACCEL_CONFIG; +/- 2G full scale select
	PORT_imuWriteRegister(0x1d, 0b00000001);	// ACCEL_CONFIG2 register; clearing ACCEL_FCHOICE_B, setting A_DLPF_CFG to 1
	PORT_imuWriteRegister(0x38, 0b11100000);	// INT_ENABLE register; enabling WoM interrupt on accelerometer
	PORT_imuWriteRegister(0x37, 0b00000000);	// INT_PIN_CFG register;
	PORT_imuWriteRegister(0x20, 0b01000000);	// ACCEL_WOM_X_THR register; threshold value for WoM
	PORT_imuWriteRegister(0x21, 0b01000000);	// ACCEL_WOM_Y_THR register;
	PORT_imuWriteRegister(0x22, 0b01000000);	// ACCEL_WOM_Z_THR register;
	PORT_imuWriteRegister(0x69, 0b11000000);	// ACCEL_INTEL_CTRL register; setting ACCEL_INTEL_EN and ACCEL_INTEL_MODE (Wake-on-Motion detection logic)
	PORT_imuWriteRegister(0x19, 0b10000000);	// SMPLRT_DIV register; dividing the sampling rate (internal sample rate = 1kHz)
	PORT_imuWriteRegister(0x6b, 0b00101001);	// PWR_MGMT_1 register; enabling low-power cycle mode for accelerometer
}

double x_degrees;
#define SAMPLE_RATE_DIV 0b00000010
#define SAMPLE_F		(double)(1000.0/(1 + SAMPLE_RATE_DIV))

void PORT_imuFifoConfig(void)
{
	PORT_imuReset();
	PORT_imuWriteRegister(0x6b, 0b00001001);		// PWR_MGMT_1 register; waking up, setting clock configuration and turning off temp. meas.
	PORT_imuWriteRegister(0x6c, 0b11000000);		// PWR_MGMT_2 register; turning on accelerometer and gyro, turning on FIFO low power mode
	PORT_imuWriteRegister(0x19, SAMPLE_RATE_DIV);	// SMPLRT_DIV register; dividing the sampling rate (internal sample rate = 1kHz)
	PORT_imuWriteRegister(0x1A, 0b01000101);		// CONFIG register;
	PORT_imuWriteRegister(0x1b, 0b00011000);		// GYRO_CONFIG register; -/+ 2000 dps
	PORT_imuWriteRegister(0x1c, 0b00000000); 		// ACCEL_CONFIG register; +/- 2G full scale select
	PORT_imuWriteRegister(0x1d, 0b11010000); 		// ACCEL_CONFIG2 register;
	PORT_imuWriteRegister(0x1e, 0b10010000);		// LP_MODE_CFG register;
	PORT_imuWriteRegister(0x23, 0b01111000);		// FIFO_EN register;
	PORT_imuWriteRegister(0x37, 0b00000000);		// INT_PIN_CFG register;
	HAL_Delay(100);
	PORT_imuSetGyroOffset();
	PORT_imuWriteRegister(0x38, 0b00010000);		// INT_ENABLE register; enabling FIFO overflow interrupt on accelerometer
	PORT_imuWriteRegister(0x6a, 0b01010100);		// USER_CTRL register; enabling and resetting FIFO

	x_degrees = 0;
	uint16_t fifo_count = 0;
	uint8_t dat[2] = { 0 };
	uint8_t fifo_data[1440] = { 0 };

	while(1)
	{
		PORT_imuReadRegister(0x72, &dat[0], 1);
		PORT_imuReadRegister(0x73, &dat[1], 1);
		fifo_count = ((0b00011111 & dat[0]) << 8) | dat[1];
		if(fifo_count > 1440)
		{
			PORT_imuReadRegister(0x74, &fifo_data[0], 1440);
			for(uint16_t i = 6; i < (1440 - 12); i+=12)
			{
				x_degrees += (double)((int16_t)((fifo_data[i] << 8) | fifo_data[i + 1]) + (int16_t)((fifo_data[i + 12] << 8) | fifo_data[i + 13]))/SAMPLE_F/2.0;
			}
		}
	}
}

inline void PORT_imuIRQHandler(void)
{
//	PORT_imuWriteRegister(0x38, 0b00000000);
	HAL_GPIO_TogglePin(LED_R1_GPIO_Port, LED_R1_Pin);
}
