#include "helper.h"
#include "libnrf24/nrf24.h"

char EMPTY_HEX[HEX_MAC_LEN] = "0000000000";

void hexlify(uint8_t* in, uint8_t size, char* out) {
    if(size > MAX_MAC_SIZE) return;

    const char hex_digits[] = "0123456789ABCDEF";

    for(int i = 0; i < size; i++) {
        out[i * 2] = hex_digits[(in[i] >> 4) & 0x0F];
        out[i * 2 + 1] = hex_digits[in[i] & 0x0F];
    }
    out[size * 2] = '\0';
}

void unhexlify(const char* in, uint8_t size, uint8_t* out) {
    if(size > MAX_MAC_SIZE) return;

    for(int i = 0; i < size; i++) {
        char high_nibble = toupper(in[i * 2]);
        char low_nibble = toupper(in[i * 2 + 1]);

        out[i] = ((high_nibble >= 'A') ? (high_nibble - 'A' + 10) : (high_nibble - '0')) << 4;
        out[i] |= (low_nibble >= 'A') ? (low_nibble - 'A' + 10) : (low_nibble - '0');
    }
}

// Function to convert the setting value to a string based on the setting type
const char* setting_value_to_string(Setting* setting, char* buffer, size_t buffer_size) {
    switch(setting->type) {
    case SETTING_TYPE_UINT8:
        snprintf(buffer, buffer_size, "%u", setting->value.u8);
        break;
    case SETTING_TYPE_UINT16:
        snprintf(buffer, buffer_size, "%u", setting->value.u16);
        break;
    case SETTING_TYPE_UINT32:
        snprintf(buffer, buffer_size, "%lu", setting->value.u32);
        break;
    case SETTING_TYPE_BOOL:
        snprintf(buffer, buffer_size, "%s", setting->value.b ? "ON" : "OFF");
        break;
    case SETTING_TYPE_DATA_RATE:
        switch(setting->value.d_r) {
        case DATA_RATE_1MBPS:
            snprintf(buffer, buffer_size, "1 Mbps");
            break;
        case DATA_RATE_2MBPS:
            snprintf(buffer, buffer_size, "2 Mbps");
            break;
        case DATA_RATE_250KBPS:
            snprintf(buffer, buffer_size, "250 kbps");
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown");
            break;
        }
        break;
    case SETTING_TYPE_TX_POWER:
        switch(setting->value.t_p) {
        case TX_POWER_M18DBM:
            snprintf(buffer, buffer_size, "-18 dBm");
            break;
        case TX_POWER_M12DBM:
            snprintf(buffer, buffer_size, "-12 dBm");
            break;
        case TX_POWER_M6DBM:
            snprintf(buffer, buffer_size, "-6 dBm");
            break;
        case TX_POWER_0DBM:
            snprintf(buffer, buffer_size, "0 dBm");
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown");
            break;
        }
        break;
    case SETTING_TYPE_ADDR_WIDTH:
        switch(setting->value.a_w) {
        case ADDR_WIDTH_2_BYTES:
            snprintf(buffer, buffer_size, "2 Bytes");
            break;
        case ADDR_WIDTH_3_BYTES:
            snprintf(buffer, buffer_size, "3 Bytes");
            break;
        case ADDR_WIDTH_4_BYTES:
            snprintf(buffer, buffer_size, "4 Bytes");
            break;
        case ADDR_WIDTH_5_BYTES:
            snprintf(buffer, buffer_size, "5 Bytes");
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown");
            break;
        }
        break;
    default:
        snprintf(buffer, buffer_size, "Unknown");
        break;
    }
    return buffer;
}

// Function to return the value of the parameter based on the type
uint32_t get_setting_value(Setting* setting) {
    switch(setting->type) {
    case SETTING_TYPE_UINT8:
        return setting->value.u8;
    case SETTING_TYPE_UINT16:
        return setting->value.u16;
    case SETTING_TYPE_UINT32:
        return setting->value.u32;
    case SETTING_TYPE_BOOL:
        return setting->value.b ? 1 : 0; // Return 1 for true, 0 for false
    case SETTING_TYPE_DATA_RATE:
        return setting->value.d_r; // Assuming uint8_t for data rate
    case SETTING_TYPE_TX_POWER:
        return setting->value.t_p; // Assuming uint8_t for TX power
    case SETTING_TYPE_ADDR_WIDTH:
        return setting->value.a_w; // Assuming uint8_t for address width
    default:
        return 0; // Unknown type
    }
}

uint8_t get_setting_index(Setting* setting) {
    uint32_t value = get_setting_value(setting);
    return (uint8_t)((value / setting->step) - (setting->min / setting->step));
}

// Function to modify the setting value based on the type
void set_setting_value(Setting* setting, uint32_t new_value) {
    switch(setting->type) {
    case SETTING_TYPE_UINT8:
        setting->value.u8 = (uint8_t)new_value;
        break;
    case SETTING_TYPE_UINT16:
        setting->value.u16 = (uint16_t)new_value;
        break;
    case SETTING_TYPE_UINT32:
        setting->value.u32 = new_value;
        break;
    case SETTING_TYPE_BOOL:
        setting->value.b = (new_value != 0); // Any non-zero value is considered true
        break;
    case SETTING_TYPE_DATA_RATE:
        setting->value.d_r = (uint8_t)new_value;
        break;
    case SETTING_TYPE_TX_POWER:
        setting->value.t_p = (uint8_t)new_value;
        break;
    case SETTING_TYPE_ADDR_WIDTH:
        setting->value.a_w = (uint8_t)new_value;
        break;
    default:
        break;
    }
}

bool is_hex_address(const char *str) {
    // Check if the string is exactly 8 characters long
    if (strlen(str) != HEX_MAC_LEN) {
        return false;
    }

    // Check if all characters are valid hexadecimal digits
    for (int i = 0; i < HEX_MAC_LEN - 1; i++) {
        if (!isxdigit((unsigned char)str[i])) {
            return false;
        }
    }

    // If all checks pass, it's a valid hexadecimal address
    return true;
}

bool is_hex_address_furi(const FuriString* furi_str) {
    // Get the length of the FuriString
    if(furi_string_size(furi_str) != HEX_MAC_LEN) {
        return false;
    }

    // Iterate over the string and check if each character is a valid hex digit
    for(size_t i = 0; i < HEX_MAC_LEN - 1; i++) {
        char c = furi_string_get_char(furi_str, i); // Get character at index 'i'
        if(!isxdigit(c)) { // Check if it's a valid hex digit (0-9, a-f, A-F)
            return false;
        }
    }

    // If all checks pass, it's a valid 8-character hexadecimal address
    return true;
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
