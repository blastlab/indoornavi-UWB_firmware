/*
 * uwb_main.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "uwb_main.h"
#include "mac/mac.h"
#include "parsers/bin_struct.h"


void Desynchronize() {
  unsigned int seed = HAL_GetTick();
  PORT_SleepMs(rand_r(&seed) % 100);
  PORT_WatchdogRefresh();
}

void TurnOff() {
  // log turn off to host
  LOG_INF("turn off");

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

void BatteryControl() {
  static unsigned int last_batt_measure_time = 0;
  if (PORT_TickMs() - last_batt_measure_time > 5000) {
    last_batt_measure_time = PORT_TickMs();

    PORT_BatteryMeasure();
    if (2400 < PORT_BatteryVoltage() && PORT_BatteryVoltage() < 3100) {
      TurnOff();
    }
  }
}

void BeaconSender() {
  static unsigned int last_beacon_time = INT32_MAX;
  if (PORT_TickMs() - last_beacon_time > 1000) {
    last_beacon_time = PORT_TickMs();
    FC_BEACON_s packet;
    packet.FC = FC_BEACON;
    packet.len = sizeof(packet);
    packet.serial = settings_otp->serial;
    mac_buf_t *buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
    MAC_Write(buf, &packet, packet.len);
    MAC_Send(buf, false);
  }
}

void RangingControl() {}

void UwbMain() {
  CheckSleepMode();
  SETTINGS_Init();
  Desynchronize();

  PORT_Init();
  MAC_Init();
  CARRY_Init(settings.mac.role == RTLS_SINK);
  FU_Init(settings.mac.role == RTLS_SINK);

  PORT_TimeStartTimers();

  while (1) {
    PORT_LedOff(LED_STAT);
    PORT_LedOff(LED_ERR);
    BatteryControl();
    RangingControl();
    BeaconSender();
    PORT_WatchdogRefresh();
  }
}
