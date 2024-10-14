#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/variable_item_list.h>
#include <storage/storage.h>
#include <notification/notification.h>
#include <stream/stream.h>
#include <furi/core/thread.h>
#include <dialogs/dialogs.h>

#include "libnrf24/nrf24.h"
#include "settings.h"

#define LOG_TAG     "NRF24TOOL"
#define MAX_CHANNEL 125
#define TOOL_QTY    4

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

typedef enum {
    MENU_RX = 1,
    MENU_TX,
    MENU_SNIFFER,
    MENU_BADMOUSE,
} MenuIndex;

typedef enum {
    VIEW_MENU = 1,
    VIEW_NRF24_NOT_CONNECTED,
    VIEW_RX_SETTINGS,
    VIEW_RX_RUN,
    VIEW_TX_SETTINGS,
    VIEW_TX_RUN,
    VIEW_SNIFF_SETTINGS,
    VIEW_SNIFF_RUN,
    VIEW_BM_SETTINGS,
    VIEW_BM_RUN,
    VIEW_BM_NO_CHANNEL,
} AppView;

/* Application context structure */
typedef struct Nrf24Tool {
    ViewDispatcher* view_dispatcher;
    Submenu* app_menu;
    VariableItemList* rx_settings;
    VariableItemList* tx_settings;
    VariableItemList* sniff_settings;
    VariableItemList* badmouse_settings;
    View* rx_run;
    View* tx_run;
    View* sniff_run;
    View* badmouse_run;
    DialogEx* nrf24_disconnect;
    DialogEx* bm_no_channel;
    FuriMessageQueue* event_queue;
    Storage* storage;
    Stream* stream;
    NotificationApp* notification;
    FuriThread* sniff_thread;
    FuriThread* badmouse_thread;
    Settings* settings;
    bool app_running;
    bool tool_running;
} Nrf24Tool;

void app_menu_enter_callback(void* context, uint32_t index);
uint32_t app_menu_exit_callback(void* context);
