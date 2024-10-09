#pragma once

#include "settings.h"
#include <furi.h>


void hexlify(uint8_t* in, uint8_t size, char* out);
const char* setting_value_to_string(Setting setting, char* buffer, size_t buffer_size);
uint32_t get_setting_value(Setting* setting);
void set_setting_value(Setting* setting, uint32_t new_value);
uint64_t bytes_to_int64(uint8_t* bytes, uint8_t size, bool bigendian);
void int64_to_bytes(uint64_t val, uint8_t* out, bool bigendian);
uint32_t bytes_to_int32(uint8_t* bytes, bool bigendian);
void int32_to_bytes(uint32_t val, uint8_t* out, bool bigendian);
uint64_t bytes_to_int16(uint8_t* bytes, bool bigendian);
void int16_to_bytes(uint16_t val, uint8_t* out, bool bigendian);
