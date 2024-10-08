#include "settings.h"

Settings nrf24Tool_settings;

SettingMapping settings_map[] = {
    {"sniff_min_channel", &nrf24Tool_settings.sniff_settings[0].value.uint8_val,  SETTING_TYPE_UINT8},
    {"sniff_max_channel", &nrf24Tool_settings.sniff_settings[1].value.uint8_val,  SETTING_TYPE_UINT8},
    {"sniff_scan_time",   &nrf24Tool_settings.sniff_settings[2].value.uint16_val,    SETTING_TYPE_UINT16},
    {"sniff_data_rate",   &nrf24Tool_settings.sniff_settings[3].value.data_rate_val,    SETTING_TYPE_DATA_RATE},
    
};