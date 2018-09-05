/*
 * port_imu.c
 *
 *  Created on: 14.04.2018
 *      Author: Dawid Peplinski
 */

#include "port.h"
#include "nrf_drv_gpiote.h"
#include "nrf_sdh.h"
#include "bin_parser.h"
#include "mac.h"

#define IMU_ACCEL_WOM_THRESHOLD 0b00001000
#define	IMU_NO_MOTION_PERIOD	30000			// When the time (in milis) run out and no motion is detected, device will go to sleep

// 		LSM6DSM registers
typedef enum {
	FUNC_CFG_ACCESS = 0x01,
	INT1_CTRL 		= 0x0d,
	WHO_AM_I 		= 0x0f,
	CTRL1_XL		= 0x10,
	CTRL3_C 		= 0x12,
	CTRL6_C			= 0x15,
	CTRL8_XL 		= 0x17,
	CTRL10_C 		= 0x19,
	WAKE_UP_SRC		= 0x1b,
	TAP_CFG			= 0x58,
	WAKE_UP_THS		= 0x5b,
	WAKE_UP_DUR		= 0x5c,
	MD1_CFG			= 0x5e,		// routing interrupts i.e. wakeup to INT1
} lsm6dsm_register_t;

typedef enum {
	SM_THS      	= 0x13,
} lsm6dsm_embedded_conf_t;


static volatile uint32_t 	motion_tick;
static volatile uint8_t		imu_sleep_mode;

static void ImuWriteRegister(lsm6dsm_register_t addr, uint8_t val) {
	uint8_t data[] = { addr & 0b0111111, val };					// adding 'write' bit
	nrf_gpio_pin_clear(IMU_SPI_SS_PIN);
	PORT_SpiTx(2, data);
	nrf_gpio_pin_set(IMU_SPI_SS_PIN);
}

static void ImuReadRegister(lsm6dsm_register_t addr, uint8_t *val, uint16_t count) {
	uint8_t m_addr = addr | 0b10000000;							// adding 'read' bit
	nrf_gpio_pin_clear(IMU_SPI_SS_PIN);
	PORT_SpiTx(1, &m_addr);
	PORT_SpiRx(count, val);
	nrf_gpio_pin_set(IMU_SPI_SS_PIN);
}

static void ImuReset(void) {
	uint8_t data = 1;
	ImuWriteRegister(CTRL3_C, data);
	PORT_SleepMs(20);
	ImuReadRegister(CTRL3_C, &data, 1);
	IASSERT(!(data & 1));				// if device holds a reset state
	ImuReadRegister(WHO_AM_I, &data, 1);
	IASSERT(data == 0x6a);
}

void PORT_ImuInit(void) {
#if !USE_DECA_DEVKIT
	if(settings.mac.role != RTLS_TAG) {
		return;
	}
	nrf_gpio_cfg_output(IMU_SPI_SS_PIN);
	nrf_gpio_pin_set(IMU_SPI_SS_PIN);
	motion_tick = 0;
	imu_sleep_mode = 0;
	ImuReset();
	ImuWriteRegister(CTRL10_C, 0b101);				// enabling FUNC_EN and SIGN_MOTION_EN
	ImuWriteRegister(INT1_CTRL, 0b01000000); 		// enabling INT1_SIGN_MOT
	ImuWriteRegister(FUNC_CFG_ACCESS, 0b10000000);	// enabling embedded functions register configuration
	ImuWriteRegister(SM_THS, 1);					// setting a significant motion detection threshold			TODO: change interrupt to wakeup feature
	ImuWriteRegister(FUNC_CFG_ACCESS, 0);			// disabling embedded functions register configuration
	ImuWriteRegister(CTRL1_XL, 0b01100000);			// enabling accel, setting ODR (hi-pwoer mode) and full-scale
#endif
}

void PORT_ImuMotionControl(void) {
#if !USE_DECA_DEVKIT
	if(settings.mac.role != RTLS_TAG)
		return;
	if((PORT_TickMs() - motion_tick) > IMU_NO_MOTION_PERIOD) {
CRITICAL(
		imu_sleep_mode = 1;
		PORT_LedOff(LED_G1);
		PORT_LedOff(LED_R1);
		TRANSCEIVER_EnterDeepSleep();		// don't need to suspend SDH - tags aren't using it
		)
// prepare to sleep
		while(imu_sleep_mode) {
			__WFI();
		}
// exit from sleep
		uint8_t *dummy_buf = malloc(256);
		TRANSCEIVER_WakeUp(dummy_buf, 256);
		MAC_Init(BIN_Parse);
		free(dummy_buf);
	}
#endif
}

void ImuIrqHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
	motion_tick = PORT_TickMs();
	imu_sleep_mode = 0;
}
