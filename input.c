#include "input.h"

void inputHandler(InputEvent* event, Nrf24Tool* context)
{
    if(event->key == InputKeyBack) {
            context->app_running = false;
            FURI_LOG_I(LOG_TAG, "back key presse");
        }
}