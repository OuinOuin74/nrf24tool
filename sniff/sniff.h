#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>

#include "../nrf24tool.h"
#include "../libnrf24/nrf24.h"
#include "../settings.h"
#include "../helper.h"

#define MAX_ADDRS                 100
#define MAX_CONFIRMED             32
#define DEFAULT_MAX_CHANNEL      85
#define DEFAULT_MIN_CHANNEL      2
#define DEFAULT_SCANTIME          4000
#define COUNT_THRESHOLD           2
#define FIND_CHANNEL_PAYLOAD_SIZE 4

typedef struct SniffStatus {
    uint8_t current_channel;
    char tested_addr[HEX_MAC_LEN];
    uint8_t addr_find_count;
    uint8_t addr_new_count;
} SniffStatus;

extern Setting sniff_defaults[SNIFF_SETTING_COUNT];
extern SniffStatus sniff_status;
extern uint8_t confirmed[MAX_CONFIRMED][5]; // first 32 confirmed addresses
extern uint8_t confirmed_idx;

int32_t nrf24_sniff(void* ctx);
void sniff_draw(Canvas* canvas, Nrf24Tool* context);
void sniff_input(InputEvent* event, Nrf24Tool* context);
