#pragma once

#include "settings.h"
#include <furi.h>

#define HEX_MAC_LEN 11
#define MAX_SETTINGS(setting) (((setting).max - (setting).min) / (setting).step + 1)

extern char EMPTY_HEX[HEX_MAC_LEN];

void hexlify(uint8_t* in, uint8_t size, char* out);
void unhexlify(const char* in, uint8_t size, uint8_t* out);
const char* setting_value_to_string(Setting setting, char* buffer, size_t buffer_size);
uint32_t get_setting_value(Setting* setting);
void set_setting_value(Setting* setting, uint32_t new_value);
bool is_hex_address(const char *str);
bool is_hex_address_furi(const FuriString* furi_str);
uint64_t bytes_to_int64(uint8_t* bytes, uint8_t size, bool bigendian);
void int64_to_bytes(uint64_t val, uint8_t* out, bool bigendian);
uint32_t bytes_to_int32(uint8_t* bytes, bool bigendian);
void int32_to_bytes(uint32_t val, uint8_t* out, bool bigendian);
uint64_t bytes_to_int16(uint8_t* bytes, bool bigendian);
void int16_to_bytes(uint16_t val, uint8_t* out, bool bigendian);
