#include "settings.h"


SettingMapping settings_map[] = {
    {"sniff_min_channel", &nrf24Tool_settings.sniff_settings.min_channel,  SETTING_TYPE_UINT8},
    {"sniff_max_channel", &nrf24Tool_settings.sniff_settings.max_channel,  SETTING_TYPE_UINT8},
    {"sniff_scan_time",   &nrf24Tool_settings.sniff_settings.scan_time,    SETTING_TYPE_UINT16},
    {"sniff_data_rate",   &nrf24Tool_settings.sniff_settings.data_rate,    SETTING_TYPE_DATA_RATE},
    // Ajouter d'autres réglages ici...
};
