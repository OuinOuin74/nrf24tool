#pragma once

#include "helpers/badmouse_hid.h"
#include "gui/modules/variable_item_list.h"
#include "helpers/ducky_script_i.h"
#include "../nrf24tool.h"
#include "../libnrf24/nrf24.h"
#include "../settings.h"
#include "../helper.h"

#define BAD_USB_APP_BASE_FOLDER        EXT_PATH("badusb")
#define BAD_USB_APP_PATH_LAYOUT_FOLDER BAD_USB_APP_BASE_FOLDER "/assets/layouts"
#define BAD_USB_APP_SCRIPT_EXTENSION   ".txt"
#define BAD_USB_APP_LAYOUT_EXTENSION   ".kl"
#define MAX_CONFIRMED_ADDR             32
#define DEFAULT_KB_LAYOUT              "en-US"

typedef struct BadMouse {
    NRF24L01_Config* config;
    VariableItemList* var_item_list;

    FuriString* file_path;
    uint8_t file_buf[FILE_BUFFER_LEN + 1];
    uint8_t buf_start;
    uint8_t buf_len;
    bool file_end;

    uint32_t defdelay;
    uint32_t stringdelay;
    uint32_t defstringdelay;
    uint16_t layout[128];

    FuriString* line;
    FuriString* line_prev;
    uint32_t repeat_cnt;
    uint8_t key_hold_nb;

    FuriString* string_print;
    size_t string_print_pos;
    BadMouseState state;

} BadMouse;

extern Setting badmouse_defaults[BADMOUSE_SETTING_COUNT];

int32_t nrf24_badmouse(void* ctx);
void badmouse_draw(Canvas* canvas, Nrf24Tool* context);
void badmouse_input(InputEvent* event, Nrf24Tool* context);