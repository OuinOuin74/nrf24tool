#pragma once

#include "gui/modules/variable_item_list.h"
#include "helpers/ducky_script_i.h"


#define BAD_USB_APP_BASE_FOLDER        EXT_PATH("badusb")
#define BAD_USB_APP_PATH_LAYOUT_FOLDER BAD_USB_APP_BASE_FOLDER "/assets/layouts"
#define BAD_USB_APP_SCRIPT_EXTENSION   ".txt"
#define BAD_USB_APP_LAYOUT_EXTENSION   ".kl"

typedef enum {
    BadMouseState_Init,
    BadMouseState_No_Channel,
    BadMouseState_Idle,
    BadMouseState_Run,
    BadMouseState_Done,
    BadMouseState_Error,
} BadMouseState;

typedef struct BadMouse {
    Nrf24Tool* context;
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