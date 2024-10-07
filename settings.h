#pragma once

#include <furi.h>

#include "libnrf24/nrf24.h"

struct Nrf24Tool;
extern struct Nrf24Tool* nrf24Tool_app;

typedef struct Sniff_settings {
    uint8_t min_channel;
    uint8_t max_channel;
    uint16_t scan_time;
    nrf24_data_rate data_rate;
} Sniff_settings;

typedef struct Settings {
    struct Sniff_settings sniff_settings;

} Settings;

typedef enum {
    SETTING_TYPE_UINT8,
    SETTING_TYPE_UINT16,
    SETTING_TYPE_UINT32,
    SETTING_TYPE_BOOL,
    SETTING_TYPE_DATA_RATE,
    SETTING_TYPE_TX_POWER,
    SETTING_TYPE_ADDR_WIDTH,
} SettingType;

typedef struct {
    const char* key;
    void* target;
    SettingType type;
} SettingMapping;

extern SettingMapping settings_map[4];
extern Settings nrf24Tool_settings;
