/*
 * port_imu.c
 *
 *  Created on: 14.04.2018
 *      Author: Dawid Peplinski
 */

#include "port.h"

extern SPI_HandleTypeDef hspi3;

void PORT_ImuWriteRegister(uint8_t addr, uint8_t val)
{
	uint8_t data[] = { addr, val };
	while(HAL_SPI_GetState(&hspi3) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, &data[0], sizeof(data), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

void PORT_ImuReadRegister(uint8_t addr, uint8_t *val, uint16_t count)
{
	uint8_t m_addr = addr | 0b10000000;
	while(HAL_SPI_GetState(&hspi3) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, &m_addr, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi3, val, count, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

void PORT_ImuReset(void)
{
	PORT_ImuWriteRegister(0x6b, 0b10000000);	// PWR_MGMT_1 register; reset
	HAL_Delay(100);
	PORT_ImuWriteRegister(0x6b, 0b00000000);	// PWR_MGMT_1 register; unreset
	uint8_t data = 0;
	PORT_ImuReadRegister(0x6b, &data, 1);
	if (data & (1<<7))						// if the device keeps reset state
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}
