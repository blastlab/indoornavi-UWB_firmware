/*
 * port_imu.c
 *
 *  Created on: 14.04.2018
 *      Author: Dawid Peplinski
 */

#include "port.h"
#include "nrfx_gpiote.h"
#include "nrf_sdh.h"
#include "bin_parser.h"
#include "FU.h"

// 		LSM6DSM registers
typedef enum {
	FUNC_CFG_ACCESS = 0x01,
	INT1_CTRL = 0x0d,
	WHO_AM_I = 0x0f,
	FIFO_CTRL1 = 0x06,
	FIFO_CTRL2 = 0x07,
	FIFO_CTRL3 = 0x08,
	FIFO_CTRL4 = 0x09,
	FIFO_CTRL5 = 0x0A,
	CTRL1_XL = 0x10,
	CTRL2_G = 0x11,
	CTRL3_C = 0x12,
	CTRL6_C = 0x15,
	CTRL8_XL = 0x17,
	CTRL10_C = 0x19,
	WAKE_UP_SRC = 0x1b,
	FIFO_STATUS1 = 0x3a,
	FIFO_STATUS2 = 0x3b,
	FIFO_STATUS3 = 0x3c,
	FIFO_STATUS4 = 0x3d,
	FIFO_DATA_OUT_L = 0x3e,
	FIFO_DATA_OUT_H = 0x3f,
	TAP_CFG = 0x58,
	WAKE_UP_THS = 0x5b,
	WAKE_UP_DUR = 0x5c,
	MD1_CFG = 0x5e,		// routing interrupts i.e. wakeup to INT1
} lsm6dsm_register_t;

typedef enum {
	SM_THS = 0x13,
} lsm6dsm_embedded_conf_t;

static volatile uint32_t motion_tick;
static volatile uint8_t imu_sleep_mode;

void PORT_ExtiInit(bool imu_available);

static void ImuWriteRegister(lsm6dsm_register_t addr, uint8_t val) {
#if IMU_EXTI_IRQ1
	uint8_t data[] = { addr & 0b0111111, val };					// adding 'write' bit
	PORT_SpiTx(data, 2, IMU_SPI_SS_PIN);
#endif
}

static void ImuReadRegister(lsm6dsm_register_t addr, uint8_t *val, uint16_t count) {
#if IMU_EXTI_IRQ1
	uint8_t m_addr = addr | 0b10000000;							// adding 'read' bit
	PORT_SpiTxRx(&m_addr, 1, val, count, IMU_SPI_SS_PIN);
#endif
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

static inline void ImuResetTimer() {
	motion_tick = PORT_TickMs();
	imu_sleep_mode = 0;
}

void ImuIrqHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  PORT_ImuIrqHandler();
}

static void ImuSetInterrupt() {
#if IMU_EXTI_IRQ1
	nrfx_gpiote_uninit();
	nrfx_gpiote_init();
	nrfx_gpiote_in_config_t imu_int_config = {
			.is_watcher = false,
			.hi_accuracy = false,
			.pull = NRF_GPIO_PIN_PULLDOWN,
			.sense = NRF_GPIOTE_POLARITY_LOTOHI,
	};
	APP_ERROR_CHECK(nrfx_gpiote_in_init(IMU_EXTI_IRQ1, &imu_int_config, ImuIrqHandler));
	nrfx_gpiote_in_event_enable(IMU_EXTI_IRQ1, true);
#endif
}

void PORT_ImuInit(bool imu_available) {
#if IMU_EXTI_IRQ1
	if (!imu_available) {
		return;
	}
	nrf_gpio_cfg_output(IMU_SPI_SS_PIN);
	nrf_gpio_pin_set(IMU_SPI_SS_PIN);
	motion_tick = PORT_TickMs();
	imu_sleep_mode = 0;
	ImuReset();
	if(settings.imu.is_enabled) {
		ImuWriteRegister(CTRL10_C, 0b101);				// enabling FUNC_EN and SIGN_MOTION_EN
		ImuWriteRegister(INT1_CTRL, 0b01000000); 		// enabling INT1_SIGN_MOT
		ImuWriteRegister(FUNC_CFG_ACCESS, 0b10000000);// enabling embedded functions register configuration
		ImuWriteRegister(SM_THS, 1);// setting a significant motion detection threshold			TODO: change interrupt to wakeup feature
		ImuWriteRegister(FUNC_CFG_ACCESS, 0);			// disabling embedded functions register configuration
		ImuWriteRegister(CTRL1_XL, 0b01100000);	// enabling accel, setting ODR (hi-pwoer mode) and full-scale
	}
#endif
}

void PORT_ImuMotionTrackingInit(bool imu_available) {
#if IMU_SPI_SS_PIN
	if (!imu_available) {
		return;
	}
	nrf_gpio_cfg_output(IMU_SPI_SS_PIN);
	nrf_gpio_pin_set(IMU_SPI_SS_PIN);
	ImuReset();

	ImuWriteRegister(CTRL3_C, 0b01000100);
	ImuWriteRegister(CTRL1_XL, 0b01100000);
	ImuWriteRegister(CTRL2_G, 0b01100000);

	ImuWriteRegister(FIFO_CTRL1, 0b00000000);
	ImuWriteRegister(FIFO_CTRL2, 0b00000000);
	ImuWriteRegister(FIFO_CTRL3, 0b00001001);
	ImuWriteRegister(FIFO_CTRL4, 0b00000000);
	ImuWriteRegister(FIFO_CTRL5, 0b00110110);


	uint8_t data[256] = { 0 };
	while(1) {
		uint8_t fifo_stat[2];
		float x_g, y_g, z_g, x_a, y_a, z_a;
		ImuReadRegister(FIFO_STATUS1, fifo_stat, 2);
		if(fifo_stat > 24) {
			ImuReadRegister(FIFO_DATA_OUT_L, data, 12);
			x_g = (int16_t)((data[1] << 8) | data[0])*8.75f;
			y_g = (int16_t)((data[3] << 8) | data[2])*8.75f;
			z_g = (int16_t)((data[5] << 8) | data[4])*8.75f;
			x_a = (int16_t)((data[7] << 8) | data[6])*0.061f;
			y_a = (int16_t)((data[9] << 8) | data[8])*0.061f;
			z_a = (int16_t)((data[11] << 8) | data[10])*0.061f;
		}
		PORT_SleepMs(1);
	}

#endif
}

void PORT_ImuMotionControl(bool sleep_enabled) {
	if (!sleep_enabled || !settings.imu.is_enabled) {
		return;
	}
	if ((int32_t)(PORT_TickMs() - motion_tick) > settings.imu.no_motion_period * 1000) {
		if (FU_IsActive()) {
			ImuResetTimer();
			return;
		}
		CRITICAL(
				imu_sleep_mode = 1;
				PORT_LedOff(LED_G1);
				PORT_LedOff(LED_R1);
				TRANSCEIVER_EnterSleep();
				// turning off GPIOTE and turning on PORT gpio interrupts
				ImuSetInterrupt();
		)
		// prepare to sleep
		// there is no need to suspend SDH - tags are not using it
		while (imu_sleep_mode) {
			__WFI();
		}
		// exit from sleep
		// reinit GPIOTE and gpio interrupts
		PORT_ExtiInit(true);
		PORT_WakeupTransceiver();
		IASSERT(dwt_readdevid() == DWT_DEVICE_ID);
		MAC_Reinit();
	}
}

void PORT_ImuIrqHandler() {
	ImuResetTimer();
}

void PORT_ImuSleep() {
	//todo:
}