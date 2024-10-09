#include "sniff.h"
#include "../helper.h"

static uint8_t user_index = 0;
static uint8_t draw_index = 0;
static const uint8_t ITEMS_ON_SCREEN = 4;
static const uint8_t start_x = 2;
static const uint8_t start_y = 14;
static const uint8_t step = 13;
static const uint8_t index_height = 12;

static void draw_settings(Canvas* canvas, Nrf24Tool* context) {
    //const size_t middle_x = canvas_width(canvas) / 2U;
    canvas_set_color(canvas, ColorBlack);

    // draw title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Mode : Sniffing");
    canvas_draw_line(canvas, 0, 10, canvas_width(canvas), 10);

    // draw settings
    char value[10];
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < SNIFF_SETTING_COUNT; i++) {
        uint8_t y = start_y + i * step;
        canvas_draw_str_aligned(
            canvas, start_x, y, AlignLeft, AlignTop, context->settings->sniff_settings[i + draw_index].name);
        setting_value_to_string(context->settings->sniff_settings[i + draw_index], value, sizeof(value));
        canvas_draw_str_aligned(canvas, 126, y, AlignRight, AlignTop, value);
    }

    // draw index
    canvas_set_color(canvas, ColorXOR);
    canvas_draw_rbox(
        canvas, 0, (start_y - 2) + (user_index - draw_index) * step, canvas_width(canvas), index_height, 0);
}

static void draw_run(Canvas* canvas, Nrf24Tool* context) {
    //const size_t middle_x = canvas_width(canvas) / 2U;
    canvas_set_color(canvas, ColorBlack);

    // draw title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Mode : Sniffing");
    canvas_draw_line(canvas, 0, 10, canvas_width(canvas), 10);

    // draw status
    canvas_set_font(canvas, FontSecondary);
    // application status
    uint8_t line_num = 0;
    if(context->tool_running)
        canvas_draw_str_aligned(canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, "Status : RUN");
    else
        canvas_draw_str_aligned(canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, "Status : IDLE");
    // current channel
    line_num++;
    char status_str[30];
    if(sniff_status.current_channel != 0) snprintf(status_str, 30, "Scanning channel : %u", sniff_status.current_channel);
    else snprintf(status_str, 30, "Testing address : %s", sniff_status.tested_addr);
    canvas_draw_str_aligned(canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, status_str);
    // find address
    line_num++;
    if(strncmp(sniff_status.last_find_addr, EMPTY_HEX, 10) == 0)
        strcpy(status_str, "Address found : NONE");
    else
        snprintf(status_str, 30, "Address found : %s", sniff_status.last_find_addr);
    canvas_draw_str_aligned(canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, status_str);
    line_num++;
    // counters
    snprintf(status_str, 30, "Found : %u    |    New : %u", sniff_status.addr_find_count, sniff_status.addr_new_count);
    canvas_draw_str_aligned(canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, status_str);
}

static void input_setting(InputEvent* event, Nrf24Tool* context) {
    Setting* setting = &context->settings->sniff_settings[user_index];
    uint32_t value = get_setting_value(setting);

    switch(event->key) {
    case InputKeyDown:
        user_index++;
        if(user_index == SNIFF_SETTING_COUNT) user_index = 0;
        break;

    case InputKeyUp:
        if(user_index > 0) user_index--;
        else user_index = SNIFF_SETTING_COUNT - 1;
        break;

    case InputKeyLeft:
        if(value >= setting->min + setting->step)
            set_setting_value(setting, value - setting->step);
        else
            set_setting_value(setting, setting->max);
        break;

    case InputKeyRight:
        value += setting->step;
        if(value > setting->max)
            set_setting_value(setting, setting->min);
        else
            set_setting_value(setting, value);
        break;

    case InputKeyOk:
        if(context->settings->sniff_settings[SNIFF_SETTING_MAX_CHANNEL].value.u8 >
           context->settings->sniff_settings[SNIFF_SETTING_MIN_CHANNEL].value.u8)
        {
            context->tool_running = true;
            context->currentMode = MODE_SNIFF_RUN;
            nrf24_sniff(context);
        }
        break;

    case InputKeyBack:
        context->app_running = false;
        break;

    default:
        break;
    }
    // adjust draw index
    if(user_index >= ITEMS_ON_SCREEN) draw_index = user_index - ITEMS_ON_SCREEN + 1;
    if(user_index < draw_index) draw_index = user_index;
}

static void input_run(InputEvent* event, Nrf24Tool* context) {

    switch(event->key) {
    case InputKeyDown:
        break;

    case InputKeyBack:
        if(context->tool_running) context->tool_running = false;
        else context->currentMode = MODE_SNIFF_SETTINGS;
        break;

    default:
        break;
    }
}

void sniff_draw(Canvas* canvas, Nrf24Tool* context) {
    if(context->currentMode == MODE_SNIFF_SETTINGS) draw_settings(canvas, context);
    if(context->currentMode == MODE_SNIFF_RUN) draw_run(canvas, context);
}

void sniff_input(InputEvent* event, Nrf24Tool* context) {
    if(context->currentMode == MODE_SNIFF_SETTINGS) input_setting(event, context);
    if(context->currentMode == MODE_SNIFF_RUN) input_run(event, context);
}
