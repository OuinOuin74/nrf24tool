#ifndef NRF24SCAN_H
#define NRF24SCAN_H

#include <gui/gui.h>
#include <gui/view_port.h>


/* Application context structure */
typedef struct
{
    Gui* gui;
    ViewPort* view_port;
    FuriThread* reader_thread;
    FuriMessageQueue* event_queue;
} NRF24scanContext;

#endif