#pragma once

#include <furi.h>
#include "nrf24tool.h"
#include "libnrf24/nrf24.h"

extern NRF24L01_Config badmouse_config;

bool find_channel(Nrf24Tool* context);
bool bm_send_key(Nrf24Tool* context, uint16_t hid_code);
void bm_release_key(Nrf24Tool* context, uint16_t hid_code);
void bm_release_all(Nrf24Tool* context);
bool bm_start_transmission(Nrf24Tool* context);
bool bm_end_transmission(Nrf24Tool* context);
