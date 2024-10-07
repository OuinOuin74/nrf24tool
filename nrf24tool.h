#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <storage/storage.h>
#include <notification/notification.h>


#include "libnrf24/nrf24.h"
#include "settings.h"


#define LOG_TAG "NRF24SCAN"
#define MAX_CHANNEL	125

enum Mode {
    MODE_RF24_DISCONNECTED,
    MODE_RX_SETTINGS,
    MODE_RX_RUN,
    MODE_TX_SETTINGS,
    MODE_TX_RUN,
    MODE_SNIFF_SETTINGS,
    MODE_SNIFF_RUN,
    MODE_BADMOUSE_SETTINGS,
    MODE_BADMOUSE_RUN
};

/* Application context structure */
typedef struct Nrf24Tool {
    Gui* gui;
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Storage* storage;
    NotificationApp* notification;
    FuriMutex* mutex;
    enum Mode currentMode;
    Settings* settings;
    bool app_running;
} Nrf24Tool;