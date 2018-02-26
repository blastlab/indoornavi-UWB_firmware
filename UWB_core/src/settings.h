#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "transceiver_settings.h"
#include "mac_settings.h"
#include "prot/carry_settings.h"

typedef struct
{
    transceiver_settings_t transceiver;
    mac_settings_t mac;
    carry_settings_t carry;
} settings_t;

extern settings_t settings;
#endif