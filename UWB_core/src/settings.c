#include "settings.h"

settings_otp_t _settings_otp = {
    .h_major = __H_MAJOR__,
    .h_minor = __H_MINOR__,
    .serial = 0,
};

static const settings_t _startup_settings
    __attribute__((section(".settings"))) = DEF_SETTINGS;

settings_t settings;
const settings_otp_t *settings_otp;

bool settings_is_otp_erased()
{
  int i;
  uint8_t *ptr = (uint8_t*)HARDWARE_OTP_ADDR;
  for(i = 0; i < sizeof(settings_otp_t); ++i)
  {
  	if(ptr[i] != 0xFF)
  	{
  		break;
  	}
  }

  // return true if otp is erased, false otherwise
  return i >= sizeof(settings_otp_t);
}

void SETTINGS_Init() {
	// variable settings
  memcpy(&settings, &_startup_settings, sizeof(settings));

  // otp settings - from flash or otp
  if(settings_is_otp_erased())
  {
		if(_settings_otp.serial == 0)
		{
			_settings_otp.serial = HARDWARE_UID_64;
		}
		settings_otp = &_settings_otp;
  }
  else
  {
  	settings_otp = (const settings_otp_t*)HARDWARE_OTP_ADDR;
  }
}

void SETTINGS_Save()
{
  PORT_WatchdogRefresh();
  CRITICAL(
    PORT_FlashErase((void*)&_startup_settings, sizeof(settings));
    PORT_WatchdogRefresh();
    PORT_FlashSave((void*)&_startup_settings, &settings, sizeof(settings));
  )
  PORT_WatchdogRefresh();
}
