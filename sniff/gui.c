#include "sniff.h"


static void draw_settings(Canvas* canvas, Nrf24Tool* ctx)
{
    //Nrf24Tool* context = ctx;
    UNUSED(ctx);
    const size_t middle_x = canvas_width(canvas) / 2U;

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, middle_x, 12, AlignCenter, AlignBottom, "Thermometer Demo");
    canvas_draw_line(canvas, 0, 16, 128, 16);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, middle_x, 30, AlignCenter, AlignBottom, "Connect thermometer");

    canvas_set_font(canvas, FontKeyboard);

}



void sniff_draw(Canvas* canvas, Nrf24Tool* ctx)
{
    Nrf24Tool* context = ctx;

    if(context->currentMode == MODE_SNIFF_SETTINGS)
        draw_settings(canvas, ctx);
}