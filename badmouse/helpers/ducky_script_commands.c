#include <furi_hal.h>
#include "ducky_script.h"
#include "ducky_script_i.h"
#include "../badmouse.h"

typedef int32_t (*DuckyCmdCallback)(BadMouse* badmouse, const char* line, int32_t param);

typedef struct {
    char* name;
    DuckyCmdCallback callback;
    int32_t param;
} DuckyCmd;

static int32_t ducky_fnc_delay(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint32_t delay_val = 0;
    bool state = ducky_get_number(line, &delay_val);
    if((state) && (delay_val > 0)) {
        return (int32_t)delay_val;
    }

    return ducky_error(badmouse, "Invalid number %s", line);
}

static int32_t ducky_fnc_defdelay(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &badmouse->defdelay);
    if(!state) {
        return ducky_error(badmouse, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_strdelay(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &badmouse->stringdelay);
    if(!state) {
        return ducky_error(badmouse, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_defstrdelay(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &badmouse->defstringdelay);
    if(!state) {
        return ducky_error(badmouse, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_string(BadMouse* badmouse, const char* line, int32_t param) {
    line = &line[ducky_get_command_len(line) + 1];
    furi_string_set_str(badmouse->string_print, line);
    if(param == 1) {
        furi_string_cat(badmouse->string_print, "\n");
    }

    if(badmouse->stringdelay == 0 &&
       badmouse->defstringdelay == 0) { // stringdelay not set - run command immediately
        bool state = ducky_string(badmouse, furi_string_get_cstr(badmouse->string_print));
        if(!state) {
            return ducky_error(badmouse, "Invalid string %s", line);
        }
    } else { // stringdelay is set - run command in thread to keep handling external events
        return SCRIPT_STATE_STRING_START;
    }

    return 0;
}

static int32_t ducky_fnc_repeat(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &badmouse->repeat_cnt);
    if((!state) || (badmouse->repeat_cnt == 0)) {
        return ducky_error(badmouse, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_sysrq(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(badmouse, line, true);
    //badmouse->hid->kb_press(badmouse->hid_inst, KEY_MOD_LEFT_ALT | HID_KEYBOARD_PRINT_SCREEN);
    bm_press_key(badmouse->config, (KEY_MOD_LEFT_ALT | HID_KEYBOARD_PRINT_SCREEN));
    //badmouse->hid->kb_press(badmouse->hid_inst, key);
    bm_press_key(badmouse->config, key);
    //badmouse->hid->release_all(badmouse->hid_inst);
    bm_release_all(badmouse->config);
    return 0;
}

static int32_t ducky_fnc_altchar(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_altchar(badmouse, line);
    if(!state) {
        return ducky_error(badmouse, "Invalid altchar %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_altstring(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_altstring(badmouse, line);
    if(!state) {
        return ducky_error(badmouse, "Invalid altstring %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_hold(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(badmouse, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(badmouse, "No keycode defined for %s", line);
    }
    badmouse->key_hold_nb++;
    if(badmouse->key_hold_nb > (HID_KB_MAX_KEYS - 1)) {
        return ducky_error(badmouse, "Too many keys are hold");
    }
    //badmouse->hid->kb_press(badmouse->hid_inst, key);
    bm_press_key(badmouse->config, key);
    return 0;
}

static int32_t ducky_fnc_release(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(badmouse, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(badmouse, "No keycode defined for %s", line);
    }
    if(badmouse->key_hold_nb == 0) {
        return ducky_error(badmouse, "No keys are hold");
    }
    badmouse->key_hold_nb--;
    //badmouse->hid->kb_release(badmouse->hid_inst, key);
    bm_release_key(badmouse->config, key);
    return 0;
}

static int32_t ducky_fnc_media(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_media_keycode_by_name(line);
    if(key == HID_CONSUMER_UNASSIGNED) {
        return ducky_error(badmouse, "No keycode defined for %s", line);
    }
    //badmouse->hid->consumer_press(badmouse->hid_inst, key);
    bm_press_key(badmouse->config, key);
    //badmouse->hid->consumer_release(badmouse->hid_inst, key);
    bm_release_key(badmouse->config, key);
    return 0;
}

static int32_t ducky_fnc_globe(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(badmouse, line, true);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(badmouse, "No keycode defined for %s", line);
    }

    //badmouse->hid->consumer_press(badmouse->hid_inst, HID_CONSUMER_FN_GLOBE);
    bm_press_key(badmouse->config, HID_CONSUMER_FN_GLOBE);
    //badmouse->hid->kb_press(badmouse->hid_inst, key);
    bm_press_key(badmouse->config, key);
    //badmouse->hid->kb_release(badmouse->hid_inst, key);
    bm_release_key(badmouse->config, key);
    //badmouse->hid->consumer_release(badmouse->hid_inst, HID_CONSUMER_FN_GLOBE);
    bm_release_key(badmouse->config, HID_CONSUMER_FN_GLOBE);
    return 0;
}

static int32_t ducky_fnc_waitforbutton(BadMouse* badmouse, const char* line, int32_t param) {
    UNUSED(param);
    UNUSED(badmouse);
    UNUSED(line);

    return SCRIPT_STATE_WAIT_FOR_BTN;
}

static const DuckyCmd ducky_commands[] = {
    {"REM", NULL, -1},
    {"ID", NULL, -1},
    {"DELAY", ducky_fnc_delay, -1},
    {"STRING", ducky_fnc_string, 0},
    {"STRINGLN", ducky_fnc_string, 1},
    {"DEFAULT_DELAY", ducky_fnc_defdelay, -1},
    {"DEFAULTDELAY", ducky_fnc_defdelay, -1},
    {"STRINGDELAY", ducky_fnc_strdelay, -1},
    {"STRING_DELAY", ducky_fnc_strdelay, -1},
    {"DEFAULT_STRING_DELAY", ducky_fnc_defstrdelay, -1},
    {"DEFAULTSTRINGDELAY", ducky_fnc_defstrdelay, -1},
    {"REPEAT", ducky_fnc_repeat, -1},
    {"SYSRQ", ducky_fnc_sysrq, -1},
    {"ALTCHAR", ducky_fnc_altchar, -1},
    {"ALTSTRING", ducky_fnc_altstring, -1},
    {"ALTCODE", ducky_fnc_altstring, -1},
    {"HOLD", ducky_fnc_hold, -1},
    {"RELEASE", ducky_fnc_release, -1},
    {"WAIT_FOR_BUTTON_PRESS", ducky_fnc_waitforbutton, -1},
    {"MEDIA", ducky_fnc_media, -1},
    {"GLOBE", ducky_fnc_globe, -1},
};

#define TAG "BadUsb"

#define WORKER_TAG TAG "Worker"

int32_t ducky_execute_cmd(BadMouse* badmouse, const char* line) {
    size_t cmd_word_len = strcspn(line, " ");
    for(size_t i = 0; i < COUNT_OF(ducky_commands); i++) {
        size_t cmd_compare_len = strlen(ducky_commands[i].name);

        if(cmd_compare_len != cmd_word_len) {
            continue;
        }

        if(strncmp(line, ducky_commands[i].name, cmd_compare_len) == 0) {
            if(ducky_commands[i].callback == NULL) {
                return 0;
            } else {
                return (ducky_commands[i].callback)(badmouse, line, ducky_commands[i].param);
            }
        }
    }

    return SCRIPT_STATE_CMD_UNKNOWN;
}
