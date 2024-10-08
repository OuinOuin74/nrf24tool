#include "sniff.h"


static void input_callback_settings(InputEvent* event, void* ctx) {
    Nrf24Tool* context = (Nrf24Tool*)ctx;
    furi_message_queue_put(context->event_queue, event, FuriWaitForever);
}

/* Draw the GUI of the application. The screen is completely redrawn during each call. */
static void draw_callback_settings(Canvas* canvas, void* ctx) {
    Nrf24Tool* context = ctx;

    switch (context->currentMode) {
        case MODE_SNIFF_SETTINGS:
            sniff_draw(canvas, context);
            break;

        default:
            break;
    }
}

static void input_callback_run(InputEvent* event, void* ctx) {
    Nrf24Tool* context = (Nrf24Tool*)ctx;
    furi_message_queue_put(context->event_queue, event, FuriWaitForever);
}

/* Draw the GUI of the application. The screen is completely redrawn during each call. */
static void draw_callback_run(Canvas* canvas, void* ctx) {
    Nrf24Tool* context = ctx;

    switch (context->currentMode) {
        case MODE_SNIFF_SETTINGS:
            sniff_draw(canvas, context);
            break;

        default:
            break;
    }
}

static void draw_settings(Canvas* canvas, Nrf24Tool* ctx)
{
    //Nrf24Tool* context = ctx;
    UNUSED(ctx);
    const size_t middle_x = canvas_width(canvas) / 2U;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Mode : Sniffing");
    canvas_draw_line(canvas, 0, 10, 128, 10);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, middle_x, 30, AlignCenter, AlignBottom, "Connect thermometer");

    canvas_set_font(canvas, FontKeyboard);

}

void sniff_gui_alloc(Nrf24Tool* context)
{
    ViewPort* view_port_settings = context->screen[Nrf24ViewSniffSettings].view_port;
    view_port_settings = view_port_alloc();
    view_port_draw_callback_set(view_port_settings, draw_callback_settings, context);
    view_port_input_callback_set(view_port_settings, input_callback_settings, context);

    ViewPort* view_port_run = context->screen[Nrf24ViewSniffRun].view_port;
    view_port_run = view_port_alloc();
    view_port_draw_callback_set(view_port_run, draw_callback_run, context);
    view_port_input_callback_set(view_port_run, input_callback_run, context);
}

void sniff_gui_free(Nrf24Tool* context)
{
    ViewPort* view_port_settings = context->screen[Nrf24ViewSniffSettings].view_port;
    view_port_enabled_set(view_port_settings, false);
    view_port_free(view_port_settings);

    ViewPort* view_port_run = context->screen[Nrf24ViewSniffRun].view_port;
    view_port_enabled_set(view_port_run, false);
    view_port_free(view_port_run);
}

void sniff_draw(Canvas* canvas, Nrf24Tool* ctx)
{
    Nrf24Tool* context = ctx;

    if(context->currentMode == MODE_SNIFF_SETTINGS)
        draw_settings(canvas, ctx);
}