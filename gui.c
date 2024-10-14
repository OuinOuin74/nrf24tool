#include "nrf24tool.h"
#include "helper.h"
#include "badmouse/badmouse.h"

void app_menu_enter_callback(void* context, uint32_t index) {
    Nrf24Tool* app = (Nrf24Tool*)context;

    switch(index) {
    case MENU_RX:
        app->currentMode = MODE_RX_SETTINGS;
        view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_RX_SETTINGS);
        break;

    case MENU_TX:
        app->currentMode = MODE_TX_SETTINGS;
        view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_TX_SETTINGS);
        break;

    case MENU_SNIFFER:
        app->currentMode = MODE_SNIFF_SETTINGS;
        view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_SNIFF_SETTINGS);
        break;

    case MENU_BADMOUSE:
        bm_read_address(app);
        bm_read_layouts(app);
        app->currentMode = MODE_BADMOUSE_SETTINGS;
        view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_BM_SETTINGS);
        break;

    default:
        return;
    }
}

uint32_t app_menu_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}
