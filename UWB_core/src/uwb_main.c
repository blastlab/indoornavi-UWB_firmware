/*
 * uwb_main.c
 *
 *  Created on: 21.03.2018
 *      Author: KarolTrzcinski
 */

#include "uwb_main.h"
#include "mac/mac.h"
#include "parsers/bin_struct.h"
#include "parsers/txt_parser.h"

void SendTurnOnMessage();
void SendTurnOffMessage(uint8_t reason);
void SendBeaconMessage();
void Desynchronize();
void CheckSleepMode();
void TurnOff();
void BatteryControl();
void diagnostic();

void BatteryControl()
{
  static unsigned int last_batt_measure_time = 0;
  if (PORT_TickMs() - last_batt_measure_time > 5000)
  {
    last_batt_measure_time = PORT_TickMs();

    PORT_BatteryMeasure();
    /*if (2400 < PORT_BatteryVoltage() && PORT_BatteryVoltage() < 3100) {
      TurnOff();
    }*/
    LOG_DBG("mV:%d", PORT_BatteryVoltage());
  }
}

void BeaconSender()
{
  if (MAC_BeaconTimerGetMs() > 5000)
  {
    if (settings.mac.role != RTLS_LISTENER)
    {
      SendBeaconMessage();
      MAC_BeaconTimerReset();
    }
  }
}

void RangingControl()
{
  static unsigned int last_time = INT32_MAX;
  if (PORT_TickMs() - last_time > 50)
  {
    last_time = PORT_TickMs();

    dev_addr_t addr = 0x8012;
    if (settings.mac.role == RTLS_SINK && settings.mac.addr != addr)
    {
      SYNC_SendPoll(addr, &addr, 1);
    }
  }
}

void UwbMain()
{
  //CheckSleepMode();
  SETTINGS_Init();
  Desynchronize(); // base on device address

  if (settings.mac.role == RTLS_DEFAULT)
  {
    settings.mac.role = RTLS_SINK;
  }

  PORT_Init();
  MAC_Init();
  CARRY_Init(settings.mac.role == RTLS_SINK);
  FU_Init(settings.mac.role == RTLS_SINK);
  ImuWomConfig();

  PORT_TimeStartTimers();
  SendTurnOnMessage();

  volatile int i = 0;
  while (1)
  {
    ++i;
    PORT_LedOff(LED_STAT);
    PORT_LedOff(LED_ERR);
    //BatteryControl(); //todo: HardFault
    RangingControl();
    BeaconSender();
    TXT_Control();
    ImuMotionControl();
    PORT_WatchdogRefresh();
    //PORT_SleepMs(10);
    //diagnostic();
  }
}

void SendTurnOnMessage()
{
  if (settings.mac.role != RTLS_LISTENER)
  {
    FC_TURN_ON_s packet;
    packet.FC = FC_TURN_ON;
    packet.len = sizeof(packet);
    mac_buf_t *buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
    MAC_Write(buf, &packet, packet.len);
    MAC_Send(buf, false);
    LOG_DBG("I send turn on - %X %c", settings.mac.addr, (char)settings.mac.role);
  }
}

void SendTurnOffMessage(uint8_t reason)
{
  FC_TURN_OFF_s packet;
  packet.FC = FC_TURN_OFF;
  packet.len = sizeof(packet);
  packet.reason = reason;
  mac_buf_t *buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
  MAC_Write(buf, &packet, packet.len);
  MAC_Send(buf, false);
}

void SendBeaconMessage()
{
  FC_BEACON_s packet;
  packet.FC = FC_BEACON;
  packet.len = sizeof(packet);
  packet.serial = settings_otp->serial;
  mac_buf_t *buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
  MAC_Write(buf, &packet, packet.len);
  MAC_Send(buf, false);
  LOG_DBG("I send beacon - %X %c", settings.mac.addr, (char)settings.mac.role);
}

void Desynchronize()
{
  unsigned int seed = settings_otp->serial;
  PORT_SleepMs(rand_r(&seed) % 100);
  PORT_WatchdogRefresh();
}

void TurnOff()
{
  // log turn off to host
  LOG_INF("turn off");
  SendTurnOffMessage(0);

  // wait for packet transmission
  PORT_SleepMs(100);

  TRANSCEIVER_EnterDeepSleep();
  PORT_LedOff(LED_R1);
  PORT_LedOff(LED_G1);

  // 3 times blink leds
  for (int i = 0; i < 3; ++i)
  {
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

void CheckSleepMode()
{
  uint32_t status_reg_val = PORT_BkpRegisterRead(STATUS_MAGIC_REG);
  if (status_reg_val == STATUS_MAGIC_NUMBER_GO_SLEEP)
  {
    while (1)
    {
      PORT_EnterStopMode();
    }
  }
}

#include "decadriver/deca_regs.h"
#include "stdarg.h"

void str_append(char *buf, size_t size, char *frm, ...)
{
  int len = strlen(buf);
  va_list arg;
  va_start(arg, frm);
  vsnprintf(buf + len, size - len, frm, arg);
  va_end(arg);
}

void diagnostic()
{
  char buf[50] = "";
  int len = sizeof(buf) / sizeof(*buf);
  decaIrqStatus_t st = decamutexon();
  uint32 status = dwt_read32bitreg(SYS_STATUS_ID); // Read status register low 32bits
  decamutexoff(st);

  if (status & SYS_STATUS_RFPLL_LL)
  {
    str_append(buf, len, " RFPLL_LL");
  }
  if (status & SYS_STATUS_CLKPLL_LL)
  {
    str_append(buf, len, " CLKPLL_LL");
  }
  if (status & SYS_STATUS_RXRSCS)
  {
    str_append(buf, len, " RXRSCS");
  }

  if (strlen(buf) > 0)
  {
    LOG_DBG(buf);
    dwt_write32bitoffsetreg(SYS_STATUS_ID, SYS_STATUS_OFFSET, SYS_STATUS_ALL_RX_ERR);
    PORT_SleepMs(1);
  }
}
