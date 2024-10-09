#include "sniff.h"
#include "../helper.h"

static uint8_t user_index = 0;
//static uint8_t draw_index = 0;
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
            canvas, start_x, y, AlignLeft, AlignTop, context->settings->sniff_settings[i].name);
        setting_value_to_string(context->settings->sniff_settings[i], value, sizeof(value));
        canvas_draw_str_aligned(canvas, 126, y, AlignRight, AlignTop, value);
    }

    // draw index
    canvas_set_color(canvas, ColorXOR);
    canvas_draw_rbox(
        canvas, 0, (start_y - 2) + user_index * step, canvas_width(canvas), index_height, 0);
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
        if(user_index > 0)
            user_index--;
        else
            user_index = SNIFF_SETTING_COUNT - 1;
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
            context->tool_running = false;
            context->currentMode = MODE_SNIFF_RUN;
        }
        break;

    case InputKeyBack:
        context->app_running = false;
        break;

    default:
        break;
    }
}

void sniff_draw(Canvas* canvas, Nrf24Tool* context) {
    if(context->currentMode == MODE_SNIFF_SETTINGS) draw_settings(canvas, context);
}

void sniff_input(InputEvent* event, Nrf24Tool* context) {
    if(context->currentMode == MODE_SNIFF_SETTINGS) input_setting(event, context);
}
