#include "badmouse.h"
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
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Mode : Bad Mouse");
    canvas_draw_line(canvas, 0, 10, canvas_width(canvas), 10);

    // draw settings
    char value[10];
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < BADMOUSE_SETTING_COUNT; i++) {
        uint8_t y = start_y + i * step;
        canvas_draw_str_aligned(
            canvas,
            start_x,
            y,
            AlignLeft,
            AlignTop,
            context->settings->badmouse_settings[i + draw_index].name);
        setting_value_to_string(
            context->settings->badmouse_settings[i + draw_index], value, sizeof(value));
        canvas_draw_str_aligned(canvas, 126, y, AlignRight, AlignTop, value);
    }

    // draw index
    canvas_set_color(canvas, ColorXOR);
    canvas_draw_rbox(
        canvas,
        0,
        (start_y - 2) + (user_index - draw_index) * step,
        canvas_width(canvas),
        index_height,
        0);
}

static void draw_run(Canvas* canvas, Nrf24Tool* context) {
    //const size_t middle_x = canvas_width(canvas) / 2U;
    canvas_set_color(canvas, ColorBlack);

    // draw title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Mode : Bad Mouse");
    canvas_draw_line(canvas, 0, 10, canvas_width(canvas), 10);

    // draw status
    canvas_set_font(canvas, FontSecondary);
    // application status
    uint8_t line_num = 0;
    if(context->tool_running)
        canvas_draw_str_aligned(
            canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, "Status : RUN");
    else
        canvas_draw_str_aligned(
            canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, "Status : IDLE");
}

static void input_setting(InputEvent* event, Nrf24Tool* context) {
    Setting* setting = &context->settings->sniff_settings[user_index];
    uint32_t value = get_setting_value(setting);

    switch(event->key) {
    case InputKeyDown:
        user_index++;
        if(user_index == BADMOUSE_SETTING_COUNT) user_index = 0;
        break;

    case InputKeyUp:
        if(user_index > 0)
            user_index--;
        else
            user_index = BADMOUSE_SETTING_COUNT - 1;
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
        
        break;

    case InputKeyBack:
        context->currentMode = MODE_MENU;
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
    case InputKeyLeft:
        break;

    case InputKeyRight:
        break;

    case InputKeyOk:
        if(!context->tool_running) {
            context->tool_running = true;
            if(furi_thread_get_state(context->badmouse_thread) == FuriThreadStateStopped)
                furi_thread_start(context->badmouse_thread);
        }
        break;

    case InputKeyBack:
        if(context->tool_running)
            context->tool_running = false;
        else {
            context->currentMode = MODE_BADMOUSE_SETTINGS;
        }
        furi_thread_join(context->badmouse_thread);
        break;

    default:
        break;
    }
}

void badmouse_draw(Canvas* canvas, Nrf24Tool* context) {
    if(context->currentMode == MODE_BADMOUSE_SETTINGS) draw_settings(canvas, context);
    if(context->currentMode == MODE_BADMOUSE_RUN) draw_run(canvas, context);
}

void badmouse_input(InputEvent* event, Nrf24Tool* context) {
    if(context->currentMode == MODE_BADMOUSE_SETTINGS) input_setting(event, context);
    if(context->currentMode == MODE_BADMOUSE_RUN) input_run(event, context);
}