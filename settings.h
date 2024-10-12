#pragma once

#include <furi.h>

#include "libnrf24/nrf24.h"

typedef enum {
    RX_SETTING_COUNT
} RxSettingIndex;

typedef enum {
    TX_SETTING_COUNT
} TxSettingIndex;

typedef enum {
    SNIFF_SETTING_MIN_CHANNEL,
    SNIFF_SETTING_MAX_CHANNEL,
    SNIFF_SETTING_SCAN_TIME,
    SNIFF_SETTING_DATA_RATE,
    SNIFF_SETTING_RPD,
    SNIFF_SETTING_COUNT
} SniffSettingIndex;

typedef enum {
    BADMOUSE_SETTING_ADDR_INDEX,
    BADMOUSE_SETTING_KB_LAYOUT,
    BADMOUSE_SETTING_DATA_RATE,
    BADMOUSE_SETTING_TX_POWER,
    BADMOUSE_SETTING_COUNT
} BadmouseSettingIndex;

#define SETTINGS_QTY \
    (RX_SETTING_COUNT + TX_SETTING_COUNT + SNIFF_SETTING_COUNT + BADMOUSE_SETTING_COUNT)

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
    SETTING_TYPE_STRING,
} SettingType;

typedef union {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    bool b;
    nrf24_data_rate d_r;
    nrf24_tx_power t_p;
    nrf24_addr_width a_w;
    char str[50];
} SettingValue;

typedef struct {
    char name[30];
    SettingType type;
    SettingValue value;
    uint32_t min;
    uint32_t max;
    uint32_t step;
} Setting;

typedef struct Settings {
    Setting rx_settings[RX_SETTING_COUNT];
    Setting tx_settings[TX_SETTING_COUNT];
    Setting sniff_settings[SNIFF_SETTING_COUNT];
    Setting badmouse_settings[BADMOUSE_SETTING_COUNT];
} Settings;

typedef struct {
    const char* key;
    void* target;
    SettingType type;
} SettingMapping;

extern SettingMapping settings_map[SETTINGS_QTY];
extern Settings nrf24Tool_settings;
