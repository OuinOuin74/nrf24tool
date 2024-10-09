#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>

#include "../nrf24tool.h"
#include "../libnrf24/nrf24.h"
#include "../settings.h"

#define MAX_ADDRS                 100
#define MAX_CONFIRMED             32
#define DEFAULT_MAX_CHANNEL      82
#define DEFAULT_MIN_CHANNEL      2
#define DEFAULT_SCANTIME          4000
#define COUNT_THRESHOLD           2
#define FIND_CHANNEL_PAYLOAD_SIZE 4

extern Setting sniff_defaults[SNIFF_SETTING_COUNT];

void nrf24_sniff(Nrf24Tool* context);
void sniff_draw(Canvas* canvas, Nrf24Tool* context);
void sniff_input(InputEvent* event, Nrf24Tool* context);
