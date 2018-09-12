/*
 * port_imu.c
 *
 *  Created on: 03.09.2018
 *      Author: DawidPeplinski
 */

#include "platform/port.h"
#include "parsers/bin_parser.h"
#include "mac/mac.h"

// 		Registers
#define PWR_MGMT_1			0x6b
#define PWR_MGMT_2			0x6c
#define ACCEL_CONFIG		0x1c
#define ACCEL_CONFIG_2		0x1d
#define INT_ENABLE			0x38
#define INT_PIN_CFG			0x37
#define ACCEL_WOM_X_THR		0x20
#define ACCEL_WOM_Y_THR		0x21
#define ACCEL_WOM_Z_THR		0x22
#define ACCEL_INTEL_CTRL	0x69
#define SMPLRT_DIV			0x19

#define CLKSEL 				1	// 0:7 - clock source
#define TEMP_DIS 			1	// 0:1 - when 1, temperature measuring is disabled
#define XYZ_GYRO_EN 		7	// 0:7 - 0bXYZ, 1 disables X/Y/Z gyro's axis
#define ACCEL_FS_SEL		0	// 0:3 - Accel full scale select
#define A_DLPF_CFG			1	// 0:7
#define WOM_INT_EN			7 	// 000/111 - disables/enables WoM interrupt in accelerometer
#define ACCEL_INTEL_EN		1	// 0:1 - enables WoM detection logic
#define ACCEL_INTEL_MODE	1	// 0:1 - enables comparing current sample with previous one
#define SMPLRT_DIV_VAL		128	// 0:255 - Divides internal sample rate (1kHz) as follows: sampling_freq = INT_SAMPLE_RATE / (1 + SMPLRT_DIV)
#define ACCEL_CYCLE			1	// 0:1 - enables accelerometer cycling

#define IMU_ACCEL_WOM_THRESHOLD 0b00001000

extern SPI_HandleTypeDef 	hspi3;
static volatile uint32_t 	motion_tick;
static volatile uint8_t		imu_sleep_mode;

static void ImuWriteRegister(uint8_t addr, uint8_t val) {
	uint8_t data[] = { addr, val };
	while(HAL_SPI_GetState(&hspi3) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, &data[0], sizeof(data), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

static void ImuReadRegister(uint8_t addr, uint8_t *val, uint16_t count) {
	uint8_t m_addr = addr | 0b10000000;
	while(HAL_SPI_GetState(&hspi3) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, &m_addr, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi3, val, count, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

static void ImuReset(void) {
	ImuWriteRegister(0x6b, 0b10000000);	// PWR_MGMT_1 register; reset
	HAL_Delay(100);
	ImuWriteRegister(0x6b, 0b00000000);	// PWR_MGMT_1 register; unreset
	uint8_t data = 0;
	ImuReadRegister(0x6b, &data, 1);
	if (data & (1<<7))						// if the device keeps reset state
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}

void PORT_ImuInit() {
	if(PORT_GetHwRole() != RTLS_TAG) {
		return;
	}
	motion_tick = 0;
	imu_sleep_mode = 0;
	ImuReset();
	ImuWriteRegister(PWR_MGMT_1, (TEMP_DIS << 3) | CLKSEL);
	ImuWriteRegister(PWR_MGMT_2, XYZ_GYRO_EN);
	ImuWriteRegister(ACCEL_CONFIG, (ACCEL_FS_SEL << 3));
	ImuWriteRegister(ACCEL_CONFIG_2, A_DLPF_CFG);
	ImuWriteRegister(INT_ENABLE, (WOM_INT_EN << 5));
	ImuWriteRegister(INT_PIN_CFG, 0);
	ImuWriteRegister(ACCEL_WOM_X_THR, IMU_ACCEL_WOM_THRESHOLD);
	ImuWriteRegister(ACCEL_WOM_Y_THR, IMU_ACCEL_WOM_THRESHOLD);
	ImuWriteRegister(ACCEL_WOM_Z_THR, IMU_ACCEL_WOM_THRESHOLD);
	ImuWriteRegister(ACCEL_INTEL_CTRL, (ACCEL_INTEL_EN << 7) | (ACCEL_INTEL_MODE << 6));
	ImuWriteRegister(SMPLRT_DIV, SMPLRT_DIV_VAL);
	ImuWriteRegister(PWR_MGMT_1, (ACCEL_CYCLE << 5) | (TEMP_DIS << 3) | CLKSEL);
}

void PORT_ImuMotionControl(bool sleep_enabled) {
	if(!sleep_enabled || !settings.imu.is_enabled) {
		return;
	}
	if((PORT_TickMs() - motion_tick) > settings.imu.no_motion_period * 1000) {
CRITICAL(
		imu_sleep_mode = 1;
		TRANSCEIVER_EnterSleep();
		PORT_PrepareSleepMode();
		)
		while(imu_sleep_mode) {
			__WFI();
		}
		PORT_ExitSleepMode();
		PORT_WakeupTransceiver();
		IASSERT(dwt_readdevid() == DWT_DEVICE_ID);
		MAC_Init(BIN_Parse);
	}
}

void PORT_ImuIrqHandler() {
	motion_tick = PORT_TickMs();
	imu_sleep_mode = 0;
}
