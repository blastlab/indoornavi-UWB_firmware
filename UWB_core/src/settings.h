#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "gitversion.h"
#include "mac/mac_settings.h"
#include "mac/carry_settings.h"
#include "transceiver_settings.h"

#define H_MAJOR(x) (0x1F & (x >> 3))
#define H_MINOR(x) (0x07 & (x >> 0))
#define H_VERSION_CALC(major, minor) ((major << 3) | (minor & 0x07))
#define H_VERSION H_VERSION_CALC(HARDWARE_MAJOR, HARDWARE_MINOR)

typedef struct __attribute__((packed)) {
  uint8_t h_major, h_minor;
  uint64_t serial;
} settings_otp_t;

// used also by bootloader
typedef struct __attribute__((packed)) {
  uint32_t boot_reserved;
  uint8_t h_version;
  uint8_t f_major;
  uint16_t f_minor;
  uint64_t f_hash;
} settings_version_t;

typedef struct {
  settings_version_t version;
  transceiver_settings_t transceiver;
  mac_settings_t mac;
  carry_settings_t carry;
} settings_t;

#define VERSION_SETTINGS_DEF                                                   \
{ \
  .boot_reserved = 0,\
	.h_version = H_VERSION,\
	.f_major = __F_MAJOR__,          \
  .f_minor = __F_MINOR__,\
	.f_hash = __F_HASH__,                                \
  }

#define DEF_SETTINGS                                                           \
  {                                                                            \
    .version = VERSION_SETTINGS_DEF, .transceiver = TRANSCEIVER_SETTINGS_DEF,  \
    .mac = MAC_SETTINGS_DEF, .carry = CARRY_SETTINGS_DEF,                      \
  }

extern settings_t settings;
extern settings_otp_t const *settings_otp;

void SETTINGS_Init();

#endif
