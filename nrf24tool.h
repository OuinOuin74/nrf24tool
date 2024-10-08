#pragma once

#include "gui/modules/variable_item_list.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>
#include <notification/notification.h>


#include "libnrf24/nrf24.h"
#include "settings.h"


#define LOG_TAG "NRF24SCAN"
#define MAX_CHANNEL	125
#define SCREEN_QTY 8

typedef enum {
    MODE_RF24_DISCONNECTED,
    MODE_RX_SETTINGS,
    MODE_RX_RUN,
    MODE_TX_SETTINGS,
    MODE_TX_RUN,
    MODE_SNIFF_SETTINGS,
    MODE_SNIFF_RUN,
    MODE_BADMOUSE_SETTINGS,
    MODE_BADMOUSE_RUN
} Mode;

typedef enum {
    Nrf24ViewRxSettings = 0,
    Nrf24ViewRxRun,
    Nrf24ViewTxSettings,
    Nrf24ViewTxRun,
    Nrf24ViewSniffSettings,
    Nrf24ViewSniffRun,
    Nrf24ViewBadMouseSettings,
    Nrf24ViewBadMouseRun,
} Nrf24Views;

typedef struct Screen {
    ViewPort* view_port;
    VariableItemList* itemList;
} Screen;

/* Application context structure */
typedef struct Nrf24Tool {
    Gui* gui;
    FuriMessageQueue* event_queue;
    ViewDispatcher* view_dispatcher;
    Screen screen[SCREEN_QTY];
    Storage* storage;
    NotificationApp* notification;
    FuriMutex* mutex;
    Mode currentMode;
    Settings* settings;
    bool app_running;
} Nrf24Tool;