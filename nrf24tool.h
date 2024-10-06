#ifndef NRF24SCAN_H
#define NRF24SCAN_H

#include <gui/gui.h>
#include <gui/view_port.h>

#define LOG_TAG "NRF24SCAN"
#define MAX_CHANNEL	125
#define MAX_ADDR	6

/* Application context structure */
typedef struct
{
    Gui* gui;
    ViewPort* view_port;
    FuriThread* reader_thread;
    FuriMessageQueue* event_queue;
} NRF24scanContext;

void hexlify(uint8_t* in, uint8_t size, char* out);

#endif