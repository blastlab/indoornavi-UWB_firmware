/*
 * uwb_main.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "uwb_main.h"
#include <stdio.h>
#include "mac/mac.h"
#include "mac/toa_routine.h"
#include "parsers/bin_struct.h"
#include "parsers/printer.h"
#include "parsers/txt_parser.h"
#include "ranging.h"

void SendTurnOnMessage();
void SendTurnOffMessage(uint8_t reason);
void SendBeaconMessage();
void Desynchronize();
void CheckSleepMode();
void TurnOff();
void BatteryControl();
void diagnostic();

void BatteryControl() {
	static unsigned int last_batt_measure_time = 0;
	if (PORT_TickMs() - last_batt_measure_time > 5000) {
		last_batt_measure_time = PORT_TickMs();

		PORT_BatteryMeasure();
		/*if (2400 < PORT_BatteryVoltage() && PORT_BatteryVoltage() < 3100) {
		 TurnOff();
		 }*/
		LOG_DBG("mV:%d", PORT_BatteryVoltage());
	}
}

void BeaconSender() {
	if (MAC_BeaconTimerGetMs() > settings.mac.beacon_period_ms) {
		if (settings.mac.role != RTLS_LISTENER) {
			SendBeaconMessage();
			MAC_BeaconTimerReset();
		}
	}
}

void RangingReader() {
	// urzyj peek, a dopiero potem pop aby nie nadpisac pomiaru podczas przetwarzania
	const measure_t* meas = TOA_MeasurePeek();
	if (meas != 0) {
		if (settings.mac.role != RTLS_SINK) {
			TOA_SendRes(meas);
		} else {
			// gdy pomiar jest z tagiem to go sledz
			if ((MIN(meas->did1, meas->did2) & ADDR_ANCHOR_FLAG) == 0) {
				CARRY_TrackTag(MIN(meas->did1, meas->did2), MAX(meas->did1, meas->did2));
			}
			// a wypisz kazdy pomiar
			PRINT_Measure(meas);
		}
		TOA_MeasurePop();
	}
}

void UwbMain() {
	// CheckSleepMode();
	SETTINGS_Init();
	PORT_Init();
	Desynchronize();  // base on device address

	if (settings.mac.role == RTLS_DEFAULT) {
		settings.mac.role = PORT_GetHwRole();
	}

#ifdef DBG
	LOG_SelfTest();
#endif

	MAC_Init(BIN_Parse);
	CARRY_Init(settings.mac.role == RTLS_SINK);
	FU_Init(settings.mac.role == RTLS_SINK);

	PORT_TimeStartTimers();
	SendTurnOnMessage();

	while (1) {
		PORT_LedOff(LED_STAT);
		PORT_LedOff(LED_ERR);
#if !USE_SLOT_TIMER
		MAC_TryTransmitFrame();
#endif
		BatteryControl();
		PORT_ImuMotionControl(settings.mac.role == RTLS_TAG);
		RANGING_Control();
		RangingReader();
		BeaconSender();
		TXT_Control();
		FU_Control();
		PORT_WatchdogRefresh();
	}
}

void SendTurnOnMessage() {
	if (settings.mac.role != RTLS_LISTENER) {
		FC_TURN_ON_s packet;
		packet.FC = FC_TURN_ON;
		packet.len = sizeof(packet);
		packet.fMinor = settings.version.f_minor;
		packet.src_did = settings.mac.addr;
		mac_buf_t* buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
		if (buf != 0) {
			MAC_Write(buf, &packet, packet.len);
			MAC_Send(buf, false);
			LOG_DBG("I send turn on - %X %c", settings.mac.addr, (char )settings.mac.role);
		}
	}
}

void SendTurnOffMessage(uint8_t reason) {
	FC_TURN_OFF_s packet;
	packet.FC = FC_TURN_OFF;
	packet.len = sizeof(packet);
	packet.reason = reason;
	mac_buf_t* buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
	if (buf != 0) {
		MAC_Write(buf, &packet, packet.len);
		MAC_Send(buf, false);
	}
}

void SendBeaconMessage() {
	int voltage = PORT_BatteryVoltage();
	voltage -= voltage > 2000 ? 2000 : voltage; // 2000 is voltage offset
	FC_BEACON_s packet;
	packet.FC = FC_BEACON;
	packet.len = sizeof(packet);
	packet.serial_hi = settings_otp->serial >> 32;
	packet.serial_lo = settings_otp->serial & UINT32_MAX;
	packet.hop_cnt_batt = (0 << 4) | ((voltage >> 8) & 0x0F);
	packet.voltage = voltage & 0xFF;
	packet.src_did = settings.mac.addr;
	mac_buf_t* buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
	if (buf != 0) {
		MAC_Write(buf, &packet, packet.len);
		MAC_Send(buf, false);
		LOG_DBG("I send beacon - %X role:%c", settings.mac.addr, (char )settings.mac.role);
	}
}

void Desynchronize() {
	unsigned int seed = settings_otp->serial;
	PORT_SleepMs(rand_r(&seed) % 100);
	PORT_WatchdogRefresh();
}

void TurnOff() {
	// log turn off to host
	LOG_INF(INF_DEVICE_TURN_OFF, settings.mac.addr);
	SendTurnOffMessage(0);

	// wait for packet transmission
	PORT_SleepMs(100);

	TRANSCEIVER_EnterDeepSleep();
	PORT_LedOff(LED_R1);
	PORT_LedOff(LED_G1);

	// 3 times blink leds
	for (int i = 0; i < 3; ++i) {
		PORT_SleepMs(300);
		PORT_LedOn(LED_R1);
		PORT_SleepMs(300);
		PORT_LedOff(LED_R1);
	}

	// disable IRQ
	__disable_irq();

	// wait forever
	// przeprowadz reset aby wylaczyc WWDG, a nastepnie
	// przejdz do trybu oszczednosci energii
	PORT_BkpRegisterWrite(STATUS_MAGIC_REG, STATUS_MAGIC_NUMBER_GO_SLEEP);
	PORT_Reboot();
	while (1)
		;
}

void CheckSleepMode() {
	uint32_t status_reg_val = PORT_BkpRegisterRead(STATUS_MAGIC_REG);
	if (status_reg_val == STATUS_MAGIC_NUMBER_GO_SLEEP) {
		while (1) {
			PORT_EnterStopMode();
		}
	}
}

#include "decadriver/deca_regs.h"
#include "stdarg.h"

void str_append(char* buf, size_t size, char* frm, ...) {
	int len = strlen(buf);
	va_list arg;
	va_start(arg, frm);
	vsnprintf(buf + len, size - len, frm, arg);
	va_end(arg);
}

void diagnostic() {
	char buf[50] = "";
	int len = sizeof(buf) / sizeof(*buf);
	decaIrqStatus_t st = decamutexon();
	uint32 status = dwt_read32bitreg(SYS_STATUS_ID);  // Read status register low 32bits
	decamutexoff(st);

	if (status & SYS_STATUS_RFPLL_LL) {
		str_append(buf, len, " RFPLL_LL");
	}
	if (status & SYS_STATUS_CLKPLL_LL) {
		str_append(buf, len, " CLKPLL_LL");
	}
	if (status & SYS_STATUS_RXRSCS) {
		str_append(buf, len, " RXRSCS");
	}

	if (strlen(buf) > 0) {
		LOG_DBG(buf);
		dwt_write32bitoffsetreg(SYS_STATUS_ID, SYS_STATUS_OFFSET,
		SYS_STATUS_ALL_RX_ERR);
		PORT_SleepMs(1);
	}
}
