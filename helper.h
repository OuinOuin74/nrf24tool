#pragma once

#include <furi.h>


void hexlify(uint8_t* in, uint8_t size, char* out);
uint64_t bytes_to_int64(uint8_t* bytes, uint8_t size, bool bigendian);
void int64_to_bytes(uint64_t val, uint8_t* out, bool bigendian);
uint32_t bytes_to_int32(uint8_t* bytes, bool bigendian);
void int32_to_bytes(uint32_t val, uint8_t* out, bool bigendian);
uint64_t bytes_to_int16(uint8_t* bytes, bool bigendian);
void int16_to_bytes(uint16_t val, uint8_t* out, bool bigendian);
