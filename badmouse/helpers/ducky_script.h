#pragma once

#include <furi.h>
#include <furi_hal.h>
#include "badmouse_hid.h"

typedef enum {
    BadMouseState_Init,
    BadMouseState_No_Channel,
    BadMouseState_Idle,
    BadMouseState_Run,
    BadMouseState_Done,
    BadMouseState_Error,
} BadMouseExecState;

typedef struct {
    BadMouseExecState state;
    size_t line_cur;
    size_t line_nb;
    uint32_t delay_remain;
    size_t error_line;
    char error[64];
} BadMouseState;

typedef struct BadMouse BadMouse;

BadMouse* bad_usb_script_open(FuriString* file_path);

void bad_usb_script_close(BadMouse* bad_usb);

void bad_usb_script_set_keyboard_layout(BadMouse* bad_usb, FuriString* layout_path);

void bad_usb_script_start(BadMouse* bad_usb);

void bad_usb_script_stop(BadMouse* bad_usb);

void bad_usb_script_start_stop(BadMouse* bad_usb);

void bad_usb_script_pause_resume(BadMouse* bad_usb);

