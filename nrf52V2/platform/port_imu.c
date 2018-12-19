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
	OUTX_L_G = 0x22,
	OUTX_L_XL = 0x28,
	MAG_OFFX_L = 0x2d,
	MAG_OFFX_H = 0x2e,
	MAG_OFFY_L = 0x2f,
	MAG_OFFY_H = 0x30,
	MAG_OFFZ_L = 0x31,
	MAG_OFFZ_H = 0x32,
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












#define				IMU_ODR						0b0100
#define 			SAMPLE_F					104
#define 			SAMPLE_COUNT			36
#define 			DEG_TO_RAD				3.14159265358979f/180.0f

#define 			deltat 					(float)(1.0f/(float)SAMPLE_F)									// sampling period in seconds
#define 			gyroMeasError 			3.14159265358979f * (5.0f / 180.0f) 			// gyroscope measurement error in rad/s (shown as 5 deg/s)
#define 			beta 					sqrt(3.0f / 4.0f) * gyroMeasError 							// compute beta

float buff_a_x, buff_a_y, buff_a_z, buff_a_a;
float r_acc_X, r_acc_Y, r_acc_Z;
float buff_w_x, buff_w_y, buff_w_z;
float SEq_1, SEq_2, SEq_3, SEq_4;
float yaw, pitch, roll;

float v_x, v_y, v_z;
float pos_x, pos_y, pos_z;

uint16_t fifo_cnt;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setQvals() {										// DEBUG function; quaternion from YPR and conversely
	roll = 0.0f;	// X
	pitch = 0.0f;	// Y
	yaw = 0.0f;		// Z
	float cy = cosf(yaw * 0.5);
	float sy = sinf(yaw * 0.5);
	float cr = cosf(roll * 0.5);
	float sr = sinf(roll * 0.5);
	float cp = cosf(pitch * 0.5);
	float sp = sinf(pitch * 0.5);

	SEq_1 = cy * cr * cp + sy * sr * sp;	// W
	SEq_2 = cy * sr * cp - sy * cr * sp;	// X
	SEq_3 = cy * cr * sp + sy * sr * cp;	// Y
	SEq_4 = sy * cr * cp - cy * sr * sp;	// Z

	roll = atan2f(2.0*(SEq_1*SEq_2 + SEq_3*SEq_4), 1 - 2.0*(SEq_2*SEq_2 + SEq_3*SEq_3));
	pitch = asinf(2.0*(SEq_1*SEq_3 - SEq_4*SEq_2));
	yaw = atan2f(2.0*(SEq_2*SEq_3 + SEq_4*SEq_1), 1 - 2.0*(SEq_3*SEq_3 + SEq_4*SEq_4));
}

//void rotate_system()	// get actual accelerations from accelerometer and YPR towards the earth; using rotation matrix from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//{
//	float X = -0.672f;		// acc_x
//	float Y = 0.441f;		// acc_y
//	float Z = 0.569f;		// acc_z
//	float yaw = 0.609f;
//	float pitch = 0.745f;
//	float roll = 0.658f;
//
//	float cy = cos(yaw);
//	float sy = sin(yaw);
//	float cp = cos(pitch);
//	float sp = sin(pitch);
//	float cr = cos(roll);
//	float sr = sin(roll);
//
//	float rx = X*cp*cy + Y*(-cr*sy + sr*sp*cy) + Z*(sr*sy + cr*sp*cy);		// actual X, Y and Z accelerations with Z as earth gradient
//	float ry = X*cp*sy + Y*(cr*cy + sr*sp*sy) + Z*(-sr*cy + cr*sp*sy);
//	float rz = X*(-sp) + Y*sr*cp + Z*cr*cp;
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ImuSetGyroOffset(void) {
//	 zeroing current offset
	uint8_t data[6] = { 0 };
	ImuWriteRegister(MAG_OFFX_L, 0);
	ImuWriteRegister(MAG_OFFX_H, 0);
	ImuWriteRegister(MAG_OFFY_L, 0);
	ImuWriteRegister(MAG_OFFY_H, 0);
	ImuWriteRegister(MAG_OFFZ_L, 0);
	ImuWriteRegister(MAG_OFFZ_H, 0);
// waiting to stabilise
	PORT_SleepMs(2000);
// signalising offset calibration
	PORT_LedOn(LED_G1);
// reading GYRO's values, setting the offset
	ImuReadRegister(OUTX_L_G, data, 6);
	int16_t x_offset = (int16_t)((data[1] << 8) | data[0]);
	int16_t y_offset = (int16_t)((data[3] << 8) | data[2]);
	int16_t z_offset = (int16_t)((data[5] << 8) | data[4]);
	x_offset = -x_offset;
	y_offset = -y_offset;
	z_offset = -z_offset;
	ImuWriteRegister(MAG_OFFX_L, (int8_t)(x_offset & 0xff));
	ImuWriteRegister(MAG_OFFX_H, (int8_t)(x_offset >> 8));
	ImuWriteRegister(MAG_OFFY_L, (int8_t)(y_offset & 0xff));
	ImuWriteRegister(MAG_OFFY_H, (int8_t)(y_offset >> 8));
	ImuWriteRegister(MAG_OFFZ_L, (int8_t)(z_offset & 0xff));
	ImuWriteRegister(MAG_OFFZ_H, (int8_t)(z_offset >> 8));
	PORT_LedOff(LED_G1);
}

static float gxFilt = 0.0;
static float gyFilt = 0.0;
static float filtAlpha = 0.3;
static inline void accFilter(void) {
	gxFilt = (1 - filtAlpha)*gxFilt + filtAlpha*buff_a_x;
	buff_a_x = gxFilt;
	gyFilt = (1 - filtAlpha)*gyFilt + filtAlpha*buff_a_y;
	buff_a_y = gyFilt;
}

static void madgwickFilterUpdate(float w_x_t, float w_y_t, float w_z_t, float a_x_t, float a_y_t, float a_z_t) {
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

static inline void rotateSystem(void) {
	roll = atan2f(2.0*(SEq_1*SEq_2 + SEq_3*SEq_4), 1 - 2.0*(SEq_2*SEq_2 + SEq_3*SEq_3));
	pitch = asinf(2.0*(SEq_1*SEq_3 - SEq_4*SEq_2));
	yaw = atan2f(2.0*(SEq_2*SEq_3 + SEq_4*SEq_1), 1 - 2.0*(SEq_3*SEq_3 + SEq_4*SEq_4));

	float cy = cosf(yaw);
	float sy = sinf(yaw);
	float cp = cosf(pitch);
	float sp = sinf(pitch);
	float cr = cosf(roll);
	float sr = sinf(roll);

	r_acc_X = buff_a_x*cp*cy + buff_a_y*(-cr*sy + sr*sp*cy) + buff_a_z*(sr*sy + cr*sp*cy);		// actual X, Y and Z accelerations with Z as earth gradient
	r_acc_Y = buff_a_x*cp*sy + buff_a_y*(cr*cy + sr*sp*sy) + buff_a_z*(-sr*cy + cr*sp*sy);
	r_acc_Z = buff_a_x*(-sp) + buff_a_y*sr*cp + buff_a_z*cr*cp;
}

static inline void integrateValues(void) {
	v_x += 9.81*100.0*r_acc_X/SAMPLE_F/2.0;
	v_y += 9.81*100.0*r_acc_Y/SAMPLE_F/2.0;

	pos_x += v_x/SAMPLE_F/2.0;
	pos_y += v_y/SAMPLE_F/2.0;
}

void setQuaternionFromAccel(void) {
	uint8_t data[6] = { 0 };
	ImuReadRegister(OUTX_L_XL, data, 6);
	float x = (int16_t)((data[1] << 8) | data[0])*0.061f;
	float y = (int16_t)((data[3] << 8) | data[2])*0.061f;
	float z = (int16_t)((data[5] << 8) | data[4])*0.061f;

	roll = atan2f(y , z);
	pitch = atan2f((-x) , sqrtf(y*y + z*z));
	yaw = 0.0f;

	float cy = cosf(yaw * 0.5);
	float sy = sinf(yaw * 0.5);
	float cr = cosf(roll * 0.5);
	float sr = sinf(roll * 0.5);
	float cp = cosf(pitch * 0.5);
	float sp = sinf(pitch * 0.5);

	SEq_1 = cy * cr * cp + sy * sr * sp;	// W
	SEq_2 = cy * sr * cp - sy * cr * sp;	// X
	SEq_3 = cy * cr * sp + sy * sr * cp;	// Y
	SEq_4 = sy * cr * cp - cy * sr * sp;	// Z
}

void PORT_ImuMotionTrackingInit(bool imu_available) {
#if IMU_SPI_SS_PIN
	if (!imu_available) {
		return;
	}
	v_x = 0.0f;	v_y = 0.0f;	v_z = 0.0f;
	pos_x = 0.0f; pos_y = 0.0f;	pos_z = 0.0f;
	nrf_gpio_cfg_output(IMU_SPI_SS_PIN);
	nrf_gpio_pin_set(IMU_SPI_SS_PIN);
	ImuReset();
	ImuWriteRegister(CTRL3_C, 0b01000100);
	ImuWriteRegister(CTRL1_XL, 0b00000000 | (IMU_ODR << 4));
	ImuWriteRegister(CTRL2_G, 0b00000000 | (IMU_ODR << 4));

	ImuSetGyroOffset();
	setQuaternionFromAccel();

	ImuWriteRegister(FIFO_CTRL1, 0b00000000);
	ImuWriteRegister(FIFO_CTRL2, 0b00000000);
	ImuWriteRegister(FIFO_CTRL3, 0b00001001);
	ImuWriteRegister(FIFO_CTRL4, 0b00000000);
	ImuWriteRegister(FIFO_CTRL5, 0b00000110 | (IMU_ODR << 3));

	uint8_t data[SAMPLE_COUNT] = { 0 };
	while(1) {
		uint8_t fifo_stat[2];
		ImuReadRegister(FIFO_STATUS1, fifo_stat, 2);
		fifo_cnt = ((fifo_stat[1] & 0b111) << 8) | fifo_stat[0];
		if(fifo_cnt > 24) {
			ImuReadRegister(FIFO_DATA_OUT_L, data, SAMPLE_COUNT);
			for(uint16_t i = 0; i < SAMPLE_COUNT; i += 12) {
				buff_w_x = (int16_t)((data[i + 1] << 8) | data[i])*0.00875f;
				buff_w_y = (int16_t)((data[i + 3] << 8) | data[i + 2])*0.00875f;
				buff_w_z = (int16_t)((data[i + 5] << 8) | data[i + 4])*0.00875f;
//				buff_a_a = sqrt(a_x * a_x + a_y * a_y + a_z * a_z);
				buff_a_x = (int16_t)((data[i + 7] << 8) | data[i + 6])*0.061f;
				buff_a_y = (int16_t)((data[i + 9] << 8) | data[i + 8])*0.061f;
				buff_a_z = (int16_t)((data[i + 11] << 8) | data[i + 10])*0.061f;
//				accFilter();
				madgwickFilterUpdate(buff_w_x*DEG_TO_RAD, buff_w_y*DEG_TO_RAD, buff_w_z*DEG_TO_RAD, buff_a_x, buff_a_y, buff_a_z);
				rotateSystem();
				integrateValues();
			}
		}
	}
#endif
}
