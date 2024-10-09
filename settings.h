#pragma once

#include <furi.h>

#include "libnrf24/nrf24.h"

typedef enum {
    SNIFF_SETTING_MIN_CHANNEL,
    SNIFF_SETTING_MAX_CHANNEL,
    SNIFF_SETTING_SCAN_TIME,
    SNIFF_SETTING_DATA_RATE,
    SNIFF_SETTING_COUNT
} SniffSettingIndex;

#define SETTINGS_RX_QTY       0
#define SETTINGS_TX_QTY       0
#define SETTINGS_BADMOUSE_QTY 0
#define SETTINGS_QTY \
    (SETTINGS_RX_QTY + SETTINGS_TX_QTY + SNIFF_SETTING_COUNT + SETTINGS_BADMOUSE_QTY)

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
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    bool b;
    nrf24_data_rate d_r;
    nrf24_tx_power t_p;
    nrf24_addr_width a_w;
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
    uint32_t min;
    uint32_t max;
    uint32_t step;
} Setting;

typedef struct Settings {
    Setting sniff_settings[SNIFF_SETTING_COUNT];

} Settings;

typedef struct {
    const char* key;
    void* target;
    SettingType type;
} SettingMapping;

extern SettingMapping settings_map[SETTINGS_QTY];
extern Settings nrf24Tool_settings;
