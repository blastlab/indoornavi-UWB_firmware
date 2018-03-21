#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "transceiver_settings.h"
#include "mac/mac_settings.h"
#include "prot/carry_settings.h"

typedef struct
{
    transceiver_settings_t transceiver;
    mac_settings_t mac;
    carry_settings_t carry;
} settings_t;

#define DEF_SETTINGS                             \
    {                                            \
        .transceiver = TRANSCEIVER_SETTINGS_DEF, \
        .mac = MAC_SETTINGS_DEF,                 \
        .carry = CARRY_SETTINGS_DEF,             \
    \
}

extern settings_t settings;
#endif
