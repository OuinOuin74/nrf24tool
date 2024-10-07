
#include "helper.h"

void hexlify(uint8_t* in, uint8_t size, char* out) {
    const char hex_digits[] = "0123456789ABCDEF";
    
    for (int i = 0; i < size; i++) {
        out[i * 2] = hex_digits[(in[i] >> 4) & 0x0F];
        out[i * 2 + 1] = hex_digits[in[i] & 0x0F];
    }
    out[size * 2] = '\0';
}

uint64_t bytes_to_int64(uint8_t* bytes, uint8_t size, bool bigendian) {
    uint64_t ret = 0;
    for(int i = 0; i < size; i++) {
        if(bigendian)
            ret |= bytes[i] << ((size - 1 - i) * 8);
        else
            ret |= bytes[i] << (i * 8);
    }

    return ret;
}

void int64_to_bytes(uint64_t val, uint8_t* out, bool bigendian) {
    for(int i = 0; i < 8; i++) {
        if(bigendian)
            out[i] = (val >> ((7 - i) * 8)) & 0xff;
        else
            out[i] = (val >> (i * 8)) & 0xff;
    }
}

uint32_t bytes_to_int32(uint8_t* bytes, bool bigendian) {
    uint32_t ret = 0;
    for(int i = 0; i < 4; i++) {
        if(bigendian)
            ret |= bytes[i] << ((3 - i) * 8);
        else
            ret |= bytes[i] << (i * 8);
    }

    return ret;
}

void int32_to_bytes(uint32_t val, uint8_t* out, bool bigendian) {
    for(int i = 0; i < 4; i++) {
        if(bigendian)
            out[i] = (val >> ((3 - i) * 8)) & 0xff;
        else
            out[i] = (val >> (i * 8)) & 0xff;
    }
}

uint64_t bytes_to_int16(uint8_t* bytes, bool bigendian) {
    uint16_t ret = 0;
    for(int i = 0; i < 2; i++) {
        if(bigendian)
            ret |= bytes[i] << ((1 - i) * 8);
        else
            ret |= bytes[i] << (i * 8);
    }

    return ret;
}

void int16_to_bytes(uint16_t val, uint8_t* out, bool bigendian) {
    for(int i = 0; i < 2; i++) {
        if(bigendian)
            out[i] = (val >> ((1 - i) * 8)) & 0xff;
        else
            out[i] = (val >> (i * 8)) & 0xff;
    }
}