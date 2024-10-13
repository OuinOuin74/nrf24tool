#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <storage/storage.h>
#include <notification/notification.h>
#include <stream/stream.h>
#include <furi/core/thread.h>
#include <dialogs/dialogs.h>

#include "libnrf24/nrf24.h"
#include "settings.h"

#define LOG_TAG     "NRF24TOOL"
#define MAX_CHANNEL 125
#define TOOL_QTY 4

typedef enum {
    MODE_MENU,
    MODE_RX_SETTINGS,
    MODE_RX_RUN,
    MODE_TX_SETTINGS,
    MODE_TX_RUN,
    MODE_SNIFF_SETTINGS,
    MODE_SNIFF_RUN,
    MODE_BADMOUSE_SETTINGS,
    MODE_BADMOUSE_RUN
} Mode;

/* Application context structure */
typedef struct Nrf24Tool {
    Gui* gui;
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Storage* storage;
    Stream* stream;
    NotificationApp* notification;
    FuriThread* sniff_thread;
    FuriThread* badmouse_thread;
    Mode currentMode;
    Settings* settings;
    bool app_running;
    bool tool_running;
} Nrf24Tool;

void draw_menu(Canvas* canvas);
void input_menu(InputEvent* event, Nrf24Tool* context);