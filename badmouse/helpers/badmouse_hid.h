#pragma once

#include <furi.h>
#include "../nrf24tool.h"
#include "libnrf24/nrf24.h"


extern NRF24L01_Config badmouse_config;

bool find_channeannel(Nrf24Tool* context);