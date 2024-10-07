#pragma once

#include "input/input.h"
#include "nrf24tool.h"
#include <furi.h>

void inputHandler(InputEvent* event, Nrf24Tool* context);