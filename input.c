#include "input.h"
#include "nrf24tool.h"
#include "sniff/sniff.h"

// handle butons
inline void handleEvent(void* context)
{
    Nrf24Tool* app = (Nrf24Tool*)context;

    InputEvent event;
    const FuriStatus status =
        furi_message_queue_get(app->event_queue, &event, 10);

    if((status != FuriStatusOk) ||
       (event.type != InputTypeShort && event.type != InputTypeRepeat)) {
        return;
    }

    // execute input fonctions
    switch(app->currentMode) {

        // Sniffing mode
        case MODE_SNIFF_SETTINGS:
        case MODE_SNIFF_RUN:
        sniff_input(&event, context);
        break;

    default:
        break;
    }
}