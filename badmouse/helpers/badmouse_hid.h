#pragma once

#include <furi.h>
#include "nrf24tool.h"
#include "libnrf24/nrf24.h"


extern NRF24L01_Config badmouse_config;

bool find_channeannel(Nrf24Tool* context);
bool send_hid_packet(Nrf24Tool* context, uint8_t mod, uint8_t hid);
bool start_transmission(Nrf24Tool* context);
bool end_transmission(Nrf24Tool* context);