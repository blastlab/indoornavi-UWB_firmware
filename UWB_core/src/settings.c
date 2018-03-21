#include "settings.h"

static const settings_t _startup_settings
    __attribute__((section(".settings"))) = DEF_SETTINGS;
settings_t settings;

void settings_init() { settings = _startup_settings; }
