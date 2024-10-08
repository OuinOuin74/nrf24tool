#pragma once

#include <furi.h>

#include "libnrf24/nrf24.h"

#define SETTINGS_RX_QTY 0
#define SETTINGS_TX_QTY 0
#define SETTINGS_SNIFF_QTY 4
#define SETTINGS_BADMOUSE_QTY 0
#define SETTINGS_QTY (SETTINGS_RX_QTY + SETTINGS_TX_QTY + SETTINGS_SNIFF_QTY + SETTINGS_BADMOUSE_QTY)

struct Nrf24Tool;
extern struct Nrf24Tool* nrf24Tool_app;

typedef enum {
    SETTING_TYPE_UINT8,
    SETTING_TYPE_UINT16,
    SETTING_TYPE_UINT32,
    SETTING_TYPE_BOOL,
    SETTING_TYPE_DATA_RATE,
    SETTING_TYPE_TX_POWER,
    SETTING_TYPE_ADDR_WIDTH,
} SettingType;

typedef union {
    uint8_t uint8_val;
    uint16_t uint16_val;
    uint32_t uint32_val;
    bool bool_val;
    nrf24_data_rate data_rate_val;
    nrf24_tx_power tx_power_val;
    nrf24_addr_width addr_width_val;
} SettingValue;

typedef enum {
    SETTING_RX = 0,
    SETTING_TX,
    SETTING_SNIFF,
    SETTING_BADMOUSE,
    SETTING_APP,
} SettingMode;

typedef struct {
    char name[30];
    SettingType type;
    SettingValue value;
} Setting;

typedef struct Sniff_settings {
    Setting min_channel;
    Setting max_channel;
    Setting scan_time;
    Setting data_rate;
} Sniff_settings;

typedef struct Settings {
    Setting sniff_settings[SETTINGS_SNIFF_QTY];

} Settings;

typedef struct {
    const char* key;
    void* target;
    SettingType type;
} SettingMapping;

extern SettingMapping settings_map[SETTINGS_QTY];
extern Settings nrf24Tool_settings;
