#include "platform/port.h"
//#include "stm32l4xx_ll_crc.h"

void PORT_BatteryInit();
void PORT_SpiInit();
void PORT_CrcInit();
void PORT_TimeInit();

void PORT_Init() {
  PORT_SpiInit();
  PORT_BatteryInit();
  PORT_CrcInit();
  PORT_TimeInit();
#if !DBG
  PORT_WatchdogInit();
#endif
}

void PORT_WatchdogInit() {

}

void PORT_WatchdogRefresh() {

}

// turn led on
void PORT_LedOn(int LED_x) {

}

// turrn led off
void PORT_LedOff(int LED_x) {

}

// reset dw 1000 device by polling RST pin down for a few ms
void PORT_ResetTransceiver() {

}

void PORT_EnterStopMode() {

}
