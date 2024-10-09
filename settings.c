#include "settings.h"

Settings nrf24Tool_settings;

SettingMapping settings_map[] = {
    {"sniff_min_channel",
     &nrf24Tool_settings.sniff_settings[SNIFF_SETTING_MIN_CHANNEL].value.u8,
     SETTING_TYPE_UINT8},
    {"sniff_max_channel",
     &nrf24Tool_settings.sniff_settings[SNIFF_SETTING_MAX_CHANNEL].value.u8,
     SETTING_TYPE_UINT8},
    {"sniff_scan_time",
     &nrf24Tool_settings.sniff_settings[SNIFF_SETTING_SCAN_TIME].value.u16,
     SETTING_TYPE_UINT16},
    {"sniff_data_rate",
     &nrf24Tool_settings.sniff_settings[SNIFF_SETTING_DATA_RATE].value.d_r,
     SETTING_TYPE_DATA_RATE},

};
