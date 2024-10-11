#pragma once

#include <furi.h>
#include "ducky_script.h"
#include "badmouse_hid.h"

#define SCRIPT_STATE_ERROR        (-1)
#define SCRIPT_STATE_END          (-2)
#define SCRIPT_STATE_NEXT_LINE    (-3)
#define SCRIPT_STATE_CMD_UNKNOWN  (-4)
#define SCRIPT_STATE_STRING_START (-5)
#define SCRIPT_STATE_WAIT_FOR_BTN (-6)

#define FILE_BUFFER_LEN 16

uint16_t ducky_get_keycode(BadMouse* bad_usb, const char* param, bool accept_chars);

uint32_t ducky_get_command_len(const char* line);

bool ducky_is_line_end(const char chr);

uint16_t ducky_get_keycode_by_name(const char* param);

uint16_t ducky_get_media_keycode_by_name(const char* param);

bool ducky_get_number(const char* param, uint32_t* val);

bool ducky_numpad_press(BadMouse* bad_usb, const char num);

bool ducky_altchar(BadMouse* bad_usb, const char* charcode);

bool ducky_altstring(BadMouse* bad_usb, const char* param);

bool ducky_string(BadMouse* bad_usb, const char* param);

int32_t ducky_execute_cmd(BadMouse* bad_usb, const char* line);

int32_t ducky_error(BadMouse* bad_usb, const char* text, ...);
