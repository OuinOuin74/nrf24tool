#pragma once

#include <furi.h>
#include "nrf24tool.h"
#include "libnrf24/nrf24.h"

#define LOGITECH_MIN_CHANNEL       2
#define LOGITECH_MAX_CHANNEL       83

uint8_t find_channel(NRF24L01_Config* config);
bool bm_press_key(NRF24L01_Config* config, uint16_t hid_code);
void bm_release_key(NRF24L01_Config* config, uint16_t hid_code);
void bm_release_all(NRF24L01_Config* config);
bool bm_send_keep_alive(NRF24L01_Config* config);
bool bm_start_transmission(NRF24L01_Config* config);
bool bm_end_transmission(NRF24L01_Config* config);
