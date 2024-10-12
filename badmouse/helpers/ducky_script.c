#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/toolbox/args.h>
#include <storage/storage.h>
#include "ducky_script.h"
#include "ducky_script_i.h"
#include <dolphin/dolphin.h>
#include "badmouse/badmouse.h"
#include "badmouse_hid.h"

#define WORKER_TAG "Badmouse"

#define BADUSB_ASCII_TO_KEY(script, x) \
    (((uint8_t)x < 128) ? (script->layout[(uint8_t)x]) : HID_KEYBOARD_NONE)

typedef enum {
    WorkerEvtStartStop = (1 << 0),
    WorkerEvtPauseResume = (1 << 1),
    WorkerEvtEnd = (1 << 2),
    WorkerEvtConnect = (1 << 3),
    WorkerEvtDisconnect = (1 << 4),
} WorkerEvtFlags;

static const uint8_t numpad_keys[10] = {
    HID_KEYPAD_0,
    HID_KEYPAD_1,
    HID_KEYPAD_2,
    HID_KEYPAD_3,
    HID_KEYPAD_4,
    HID_KEYPAD_5,
    HID_KEYPAD_6,
    HID_KEYPAD_7,
    HID_KEYPAD_8,
    HID_KEYPAD_9,
};

uint32_t ducky_get_command_len(const char* line) {
    uint32_t len = strlen(line);
    for(uint32_t i = 0; i < len; i++) {
        if(line[i] == ' ') return i;
    }
    return 0;
}

bool ducky_is_line_end(const char chr) {
    return (chr == ' ') || (chr == '\0') || (chr == '\r') || (chr == '\n');
}

uint16_t ducky_get_keycode(BadMouse* badmouse, const char* param, bool accept_chars) {
    uint16_t keycode = ducky_get_keycode_by_name(param);
    if(keycode != HID_KEYBOARD_NONE) {
        return keycode;
    }

    if((accept_chars) && (strlen(param) > 0)) {
        return BADUSB_ASCII_TO_KEY(badmouse, param[0]) & 0xFF;
    }
    return 0;
}

/* bool ducky_get_number(const char* param, uint32_t* val) {
    uint32_t value = 0;
    if(strint_to_uint32(param, NULL, &value, 10) == StrintParseNoError) {
        *val = value;
        return true;
    }
    return false;
} */

bool ducky_get_number(const char* param, uint32_t* val) {
    char* endptr;
    unsigned long value = strtoul(param, &endptr, 10);

    if(*endptr == '\0' && value <= UINT32_MAX) {
        *val = (uint32_t)value;
        return true;
    }
    return false;
}

bool ducky_numpad_press(BadMouse* badmouse, const char num) {
    if((num < '0') || (num > '9')) return false;

    uint16_t key = numpad_keys[num - '0'];
    //badmouse->hid->kb_press(badmouse->hid_inst, key);
    bm_press_key(badmouse->config, key);
    //badmouse->hid->kb_release(badmouse->hid_inst, key);
    bm_release_key(badmouse->config, key);

    return true;
}

bool ducky_altchar(BadMouse* badmouse, const char* charcode) {
    uint8_t i = 0;
    bool state = false;

    //badmouse->hid->kb_press(badmouse->hid_inst, KEY_MOD_LEFT_ALT);
    bm_press_key(badmouse->config, KEY_MOD_LEFT_ALT);

    while(!ducky_is_line_end(charcode[i])) {
        state = ducky_numpad_press(badmouse, charcode[i]);
        if(state == false) break;
        i++;
    }

    //badmouse->hid->kb_release(badmouse->hid_inst, KEY_MOD_LEFT_ALT);
    bm_release_key(badmouse->config, KEY_MOD_LEFT_ALT);
    return state;
}

bool ducky_altstring(BadMouse* badmouse, const char* param) {
    uint32_t i = 0;
    bool state = false;

    while(param[i] != '\0') {
        if((param[i] < ' ') || (param[i] > '~')) {
            i++;
            continue; // Skip non-printable chars
        }

        char temp_str[4];
        snprintf(temp_str, 4, "%u", param[i]);

        state = ducky_altchar(badmouse, temp_str);
        if(state == false) break;
        i++;
    }
    return state;
}

int32_t ducky_error(BadMouse* badmouse, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(badmouse->state.error, sizeof(badmouse->state.error), text, args);

    va_end(args);
    return SCRIPT_STATE_ERROR;
}

bool ducky_string(BadMouse* badmouse, const char* param) {
    uint32_t i = 0;

    while(param[i] != '\0') {
        if(param[i] != '\n') {
            uint16_t keycode = BADUSB_ASCII_TO_KEY(badmouse, param[i]);
            if(keycode != HID_KEYBOARD_NONE) {
                //badmouse->hid->kb_press(badmouse->hid_inst, keycode);
                bm_press_key(badmouse->config, keycode);
                //badmouse->hid->kb_release(badmouse->hid_inst, keycode);
                bm_release_key(badmouse->config, keycode);
            }
        } else {
            //badmouse->hid->kb_press(badmouse->hid_inst, HID_KEYBOARD_RETURN);
            bm_press_key(badmouse->config, HID_KEYBOARD_RETURN);
            //badmouse->hid->kb_release(badmouse->hid_inst, HID_KEYBOARD_RETURN);
            bm_release_key(badmouse->config, HID_KEYBOARD_RETURN);
        }
        i++;
    }
    badmouse->stringdelay = 0;
    return true;
}

/* static bool ducky_string_next(BadMouse* badmouse) {
    if(badmouse->string_print_pos >= furi_string_size(badmouse->string_print)) {
        return true;
    }

    char print_char = furi_string_get_char(badmouse->string_print, badmouse->string_print_pos);

    if(print_char != '\n') {
        uint16_t keycode = BADUSB_ASCII_TO_KEY(badmouse, print_char);
        if(keycode != HID_KEYBOARD_NONE) {
            //badmouse->hid->kb_press(badmouse->hid_inst, keycode);
            bm_press_key(badmouse->context, keycode);
            //badmouse->hid->kb_release(badmouse->hid_inst, keycode);
            bm_release_key(badmouse->context, keycode);
        }
    } else {
        //badmouse->hid->kb_press(badmouse->hid_inst, HID_KEYBOARD_RETURN);
        bm_press_key(badmouse->context, HID_KEYBOARD_RETURN);
        //badmouse->hid->kb_release(badmouse->hid_inst, HID_KEYBOARD_RETURN);
        bm_release_key(badmouse->context, HID_KEYBOARD_RETURN);
    }

    badmouse->string_print_pos++;

    return false;
} */

/* static int32_t ducky_parse_line(BadMouse* badmouse, FuriString* line) {
    uint32_t line_len = furi_string_size(line);
    const char* line_tmp = furi_string_get_cstr(line);

    if(line_len == 0) {
        return SCRIPT_STATE_NEXT_LINE; // Skip empty lines
    }
    FURI_LOG_D(WORKER_TAG, "line:%s", line_tmp);

    // Ducky Lang Functions
    int32_t cmd_result = ducky_execute_cmd(badmouse, line_tmp);
    if(cmd_result != SCRIPT_STATE_CMD_UNKNOWN) {
        return cmd_result;
    }

    // Special keys + modifiers
    uint16_t key = ducky_get_keycode(badmouse, line_tmp, false);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(badmouse, "No keycode defined for %s", line_tmp);
    }
    if((key & 0xFF00) != 0) {
        // It's a modifier key
        line_tmp = &line_tmp[ducky_get_command_len(line_tmp) + 1];
        key |= ducky_get_keycode(badmouse, line_tmp, true);
    }
    //badmouse->hid->kb_press(badmouse->hid_inst, key);
    bm_press_key(badmouse->context, key);
    //badmouse->hid->kb_release(badmouse->hid_inst, key);
    bm_release_key(badmouse->context, key);
    return 0;
} */

/* static int32_t ducky_script_execute_next(BadMouse* badmouse, File* script_file) {
    int32_t delay_val = 0;

    if(badmouse->repeat_cnt > 0) {
        badmouse->repeat_cnt--;
        delay_val = ducky_parse_line(badmouse, badmouse->line_prev);
        if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
            return 0;
        } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
            return delay_val;
        } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
            return delay_val;
        } else if(delay_val < 0) { // Script error
            badmouse->st.error_line = badmouse->st.line_cur - 1;
            FURI_LOG_E(WORKER_TAG, "Unknown command at line %zu", badmouse->st.line_cur - 1U);
            return SCRIPT_STATE_ERROR;
        } else {
            return delay_val + badmouse->defdelay;
        }
    }

    furi_string_set(badmouse->line_prev, badmouse->line);
    furi_string_reset(badmouse->line);

    while(1) {
        if(badmouse->buf_len == 0) {
            badmouse->buf_len = storage_file_read(script_file, badmouse->file_buf, FILE_BUFFER_LEN);
            if(storage_file_eof(script_file)) {
                if((badmouse->buf_len < FILE_BUFFER_LEN) && (badmouse->file_end == false)) {
                    badmouse->file_buf[badmouse->buf_len] = '\n';
                    badmouse->buf_len++;
                    badmouse->file_end = true;
                }
            }

            badmouse->buf_start = 0;
            if(badmouse->buf_len == 0) return SCRIPT_STATE_END;
        }
        for(uint8_t i = badmouse->buf_start; i < (badmouse->buf_start + badmouse->buf_len); i++) {
            if(badmouse->file_buf[i] == '\n' && furi_string_size(badmouse->line) > 0) {
                badmouse->st.line_cur++;
                badmouse->buf_len = badmouse->buf_len + badmouse->buf_start - (i + 1);
                badmouse->buf_start = i + 1;
                furi_string_trim(badmouse->line);
                delay_val = ducky_parse_line(badmouse, badmouse->line);
                if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
                    return 0;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
                    return delay_val;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
                    return delay_val;
                } else if(delay_val < 0) {
                    badmouse->st.error_line = badmouse->st.line_cur;
                    FURI_LOG_E(WORKER_TAG, "Unknown command at line %zu", badmouse->st.line_cur);
                    return SCRIPT_STATE_ERROR;
                } else {
                    return delay_val + badmouse->defdelay;
                }
            } else {
                furi_string_push_back(badmouse->line, badmouse->file_buf[i]);
            }
        }
        badmouse->buf_len = 0;
        if(badmouse->file_end) return SCRIPT_STATE_END;
    }

    return 0;
} */
/* 
static int32_t badmouse_worker(void* context) {
    BadMouse* badmouse = context;

    BadUsbWorkerState worker_state = BadUsbStateInit;
    BadUsbWorkerState pause_state = BadUsbStateRunning;
    int32_t delay_val = 0;

    FURI_LOG_I(WORKER_TAG, "Init");
    File* script_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    badmouse->line = furi_string_alloc();
    badmouse->line_prev = furi_string_alloc();
    badmouse->string_print = furi_string_alloc();

    while(1) {
        if(worker_state == BadUsbStateInit) { // State: initialization
            if(storage_file_open(
                   script_file,
                   furi_string_get_cstr(badmouse->file_path),
                   FSAM_READ,
                   FSOM_OPEN_EXISTING)) {
                if((ducky_script_preload(badmouse, script_file)) && (badmouse->st.line_nb > 0)) {
                    if(badmouse->hid->is_connected(badmouse->hid_inst)) {
                        worker_state = BadUsbStateIdle; // Ready to run
                    } else {
                        worker_state = BadUsbStateNotConnected; // USB not connected
                    }
                } else {
                    worker_state = BadUsbStateScriptError; // Script preload error
                }
            } else {
                FURI_LOG_E(WORKER_TAG, "File open error");
                worker_state = BadUsbStateFileError; // File open error
            }
            badmouse->st.state = worker_state;

        } else if(worker_state == BadUsbStateNotConnected) { // State: USB not connected
            uint32_t flags = badmouse_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtDisconnect | WorkerEvtStartStop,
                FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) {
                worker_state = BadUsbStateIdle; // Ready to run
            } else if(flags & WorkerEvtStartStop) {
                worker_state = BadUsbStateWillRun; // Will run when USB is connected
            }
            badmouse->st.state = worker_state;

        } else if(worker_state == BadUsbStateIdle) { // State: ready to start
            uint32_t flags = badmouse_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtDisconnect, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtStartStop) { // Start executing script
                dolphin_deed(DolphinDeedBadUsbPlayScript);
                delay_val = 0;
                badmouse->buf_len = 0;
                badmouse->st.line_cur = 0;
                badmouse->defdelay = 0;
                badmouse->stringdelay = 0;
                badmouse->defstringdelay = 0;
                badmouse->repeat_cnt = 0;
                badmouse->key_hold_nb = 0;
                badmouse->file_end = false;
                storage_file_seek(script_file, 0, true);
                worker_state = BadUsbStateRunning;
            } else if(flags & WorkerEvtDisconnect) {
                worker_state = BadUsbStateNotConnected; // USB disconnected
            }
            badmouse->st.state = worker_state;

        } else if(worker_state == BadUsbStateWillRun) { // State: start on connection
            uint32_t flags = badmouse_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtStartStop, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) { // Start executing script
                dolphin_deed(DolphinDeedBadUsbPlayScript);
                delay_val = 0;
                badmouse->buf_len = 0;
                badmouse->st.line_cur = 0;
                badmouse->defdelay = 0;
                badmouse->stringdelay = 0;
                badmouse->defstringdelay = 0;
                badmouse->repeat_cnt = 0;
                badmouse->file_end = false;
                storage_file_seek(script_file, 0, true);
                // extra time for PC to recognize Flipper as keyboard
                flags = furi_thread_flags_wait(
                    WorkerEvtEnd | WorkerEvtDisconnect | WorkerEvtStartStop,
                    FuriFlagWaitAny | FuriFlagNoClear,
                    1500);
                if(flags == (unsigned)FuriFlagErrorTimeout) {
                    // If nothing happened - start script execution
                    worker_state = BadUsbStateRunning;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadUsbStateIdle;
                    furi_thread_flags_clear(WorkerEvtStartStop);
                }
            } else if(flags & WorkerEvtStartStop) { // Cancel scheduled execution
                worker_state = BadUsbStateNotConnected;
            }
            badmouse->st.state = worker_state;

        } else if(worker_state == BadUsbStateRunning) { // State: running
            uint16_t delay_cur = (delay_val > 1000) ? (1000) : (delay_val);
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriFlagWaitAny,
                delay_cur);

            delay_val -= delay_cur;
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadUsbStateIdle; // Stop executing script
                    badmouse->hid->release_all(badmouse->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadUsbStateNotConnected; // USB disconnected
                    badmouse->hid->release_all(badmouse->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    pause_state = BadUsbStateRunning;
                    worker_state = BadUsbStatePaused; // Pause
                }
                badmouse->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                if(delay_val > 0) {
                    badmouse->st.delay_remain--;
                    continue;
                }
                badmouse->st.state = BadUsbStateRunning;
                delay_val = ducky_script_execute_next(badmouse, script_file);
                if(delay_val == SCRIPT_STATE_ERROR) { // Script error
                    delay_val = 0;
                    worker_state = BadUsbStateScriptError;
                    badmouse->st.state = worker_state;
                    badmouse->hid->release_all(badmouse->hid_inst);
                } else if(delay_val == SCRIPT_STATE_END) { // End of script
                    delay_val = 0;
                    worker_state = BadUsbStateIdle;
                    badmouse->st.state = BadUsbStateDone;
                    badmouse->hid->release_all(badmouse->hid_inst);
                    continue;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Start printing string with delays
                    delay_val = badmouse->defdelay;
                    badmouse->string_print_pos = 0;
                    worker_state = BadUsbStateStringDelay;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // set state to wait for user input
                    worker_state = BadUsbStateWaitForBtn;
                    badmouse->st.state = BadUsbStateWaitForBtn; // Show long delays
                } else if(delay_val > 1000) {
                    badmouse->st.state = BadUsbStateDelay; // Show long delays
                    badmouse->st.delay_remain = delay_val / 1000;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(worker_state == BadUsbStateWaitForBtn) { // State: Wait for button Press
            uint32_t flags = badmouse_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriWaitForever);
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    delay_val = 0;
                    worker_state = BadUsbStateRunning;
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadUsbStateNotConnected; // USB disconnected
                    badmouse->hid->release_all(badmouse->hid_inst);
                }
                badmouse->st.state = worker_state;
                continue;
            }
        } else if(worker_state == BadUsbStatePaused) { // State: Paused
            uint32_t flags = badmouse_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriWaitForever);
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadUsbStateIdle; // Stop executing script
                    badmouse->st.state = worker_state;
                    badmouse->hid->release_all(badmouse->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadUsbStateNotConnected; // USB disconnected
                    badmouse->st.state = worker_state;
                    badmouse->hid->release_all(badmouse->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    if(pause_state == BadUsbStateRunning) {
                        if(delay_val > 0) {
                            badmouse->st.state = BadUsbStateDelay;
                            badmouse->st.delay_remain = delay_val / 1000;
                        } else {
                            badmouse->st.state = BadUsbStateRunning;
                            delay_val = 0;
                        }
                        worker_state = BadUsbStateRunning; // Resume
                    } else if(pause_state == BadUsbStateStringDelay) {
                        badmouse->st.state = BadUsbStateRunning;
                        worker_state = BadUsbStateStringDelay; // Resume
                    }
                }
                continue;
            }
        } else if(worker_state == BadUsbStateStringDelay) { // State: print string with delays
            uint32_t delay = (badmouse->stringdelay == 0) ? badmouse->defstringdelay :
                                                           badmouse->stringdelay;
            uint32_t flags = badmouse_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                delay);

            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = BadUsbStateIdle; // Stop executing script
                    badmouse->hid->release_all(badmouse->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = BadUsbStateNotConnected; // USB disconnected
                    badmouse->hid->release_all(badmouse->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    pause_state = BadUsbStateStringDelay;
                    worker_state = BadUsbStatePaused; // Pause
                }
                badmouse->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                bool string_end = ducky_string_next(badmouse);
                if(string_end) {
                    badmouse->stringdelay = 0;
                    worker_state = BadUsbStateRunning;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(
            (worker_state == BadUsbStateFileError) ||
            (worker_state == BadUsbStateScriptError)) { // State: error
            uint32_t flags =
                badmouse_flags_get(WorkerEvtEnd, FuriWaitForever); // Waiting for exit command

            if(flags & WorkerEvtEnd) {
                break;
            }
        }
    }

    badmouse->hid->set_state_callback(badmouse->hid_inst, NULL, NULL);
    badmouse->hid->deinit(badmouse->hid_inst);

    storage_file_close(script_file);
    storage_file_free(script_file);
    furi_string_free(badmouse->line);
    furi_string_free(badmouse->line_prev);
    furi_string_free(badmouse->string_print);

    FURI_LOG_I(WORKER_TAG, "End");

    return 0;
}
 */
static void badmouse_script_set_default_keyboard_layout(BadMouse* badmouse) {
    furi_assert(badmouse);
    memset(badmouse->layout, HID_KEYBOARD_NONE, sizeof(badmouse->layout));
    memcpy(badmouse->layout, hid_asciimap, MIN(sizeof(hid_asciimap), sizeof(badmouse->layout)));
}

BadMouse* badmouse_script_open(FuriString* file_path) {
    furi_assert(file_path);

    BadMouse* badmouse = malloc(sizeof(BadMouse));
    badmouse->file_path = furi_string_alloc();
    furi_string_set(badmouse->file_path, file_path);
    badmouse_script_set_default_keyboard_layout(badmouse);

    badmouse->state.state = BadMouseState_Init;
    badmouse->state.error[0] = '\0';

    return badmouse;
} //-V773

void badmouse_script_close(BadMouse* badmouse) {
    furi_assert(badmouse);

    furi_string_free(badmouse->file_path);
    free(badmouse);
}

void badmouse_script_set_keyboard_layout(BadMouse* badmouse, FuriString* layout_path) {
    furi_assert(badmouse);

    if(badmouse->state.state == BadMouseState_Run) {
        // do not update keyboard layout while a script is running
        return;
    }

    File* layout_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    if(!furi_string_empty(layout_path)) { //-V1051
        if(storage_file_open(
               layout_file, furi_string_get_cstr(layout_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            uint16_t layout[128];
            if(storage_file_read(layout_file, layout, sizeof(layout)) == sizeof(layout)) {
                memcpy(badmouse->layout, layout, sizeof(layout));
            }
        }
        storage_file_close(layout_file);
    } else {
        badmouse_script_set_default_keyboard_layout(badmouse);
    }
    storage_file_free(layout_file);
}
