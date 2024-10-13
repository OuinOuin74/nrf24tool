#include "nrf24tool.h"
#include "helper.h"
#include "badmouse/badmouse.h"

static uint8_t user_index = 0;
static uint8_t draw_index = 0;
static const uint8_t ITEMS_ON_SCREEN = 4;
static const uint8_t start_x = 2;
static const uint8_t start_y = 14;
static const uint8_t step = 13;
static const uint8_t index_height = 12;

void draw_menu(Canvas* canvas) {
    const size_t middle_x = canvas_width(canvas) / 2U;
    canvas_set_color(canvas, ColorBlack);

    // draw title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, middle_x, 0, AlignCenter, AlignTop, "Select Tool");
    canvas_draw_line(canvas, 0, 10, canvas_width(canvas), 10);

    // draw menu
    canvas_set_font(canvas, FontSecondary);
    uint8_t line = 0;
    canvas_draw_str_aligned(
        canvas, start_x, start_y + (line * step), AlignLeft, AlignTop, "Reception");
    line++;
    canvas_draw_str_aligned(
        canvas, start_x, start_y + (line * step), AlignLeft, AlignTop, "Emission");
    line++;
    canvas_draw_str_aligned(
        canvas, start_x, start_y + (line * step), AlignLeft, AlignTop, "Sniffer");
    line++;
    canvas_draw_str_aligned(
        canvas, start_x, start_y + (line * step), AlignLeft, AlignTop, "Bad Mouse");

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

void input_menu(InputEvent* event, Nrf24Tool* context) {
    switch(event->key) {
    case InputKeyDown:
        user_index++;
        if(user_index == TOOL_QTY) user_index = 0;
        break;

    case InputKeyUp:
        if(user_index > 0)
            user_index--;
        else
            user_index = TOOL_QTY - 1;
        break;

    case InputKeyOk:
        // change active mode
        context->currentMode = (Mode)(user_index * 2 + 1);
        if(context->currentMode == MODE_BADMOUSE_SETTINGS) {
            bm_read_address(context);
            bm_read_layouts(context);
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
