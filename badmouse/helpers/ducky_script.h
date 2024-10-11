#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>
#include "badmouse_hid.h"

typedef enum {
    BadUsbStateInit,
    BadUsbStateNotConnected,
    BadUsbStateIdle,
    BadUsbStateWillRun,
    BadUsbStateRunning,
    BadUsbStateDelay,
    BadUsbStateStringDelay,
    BadUsbStateWaitForBtn,
    BadUsbStatePaused,
    BadUsbStateDone,
    BadUsbStateScriptError,
    BadUsbStateFileError,
} BadUsbWorkerState;

typedef struct {
    BadUsbWorkerState state;
    size_t line_cur;
    size_t line_nb;
    uint32_t delay_remain;
    size_t error_line;
    char error[64];
} BadUsbState;

typedef struct BadMouse BadMouse;

BadMouse* bad_usb_script_open(FuriString* file_path);

void bad_usb_script_close(BadMouse* bad_usb);

void bad_usb_script_set_keyboard_layout(BadMouse* bad_usb, FuriString* layout_path);

void bad_usb_script_start(BadMouse* bad_usb);

void bad_usb_script_stop(BadMouse* bad_usb);

void bad_usb_script_start_stop(BadMouse* bad_usb);

void bad_usb_script_pause_resume(BadMouse* bad_usb);

BadUsbState* bad_usb_script_get_state(BadMouse* bad_usb);

#ifdef __cplusplus
}
#endif
