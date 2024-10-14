#include <stream/stream.h>
#include "stream/file_stream.h"

#include "helper.h"
#include "nrf24tool.h"
#include "libnrf24/nrf24.h"
#include "sniff/sniff.h"
#include "badmouse/badmouse.h"
#include "settings.h"

// Main app variable
Nrf24Tool* nrf24Tool_app = NULL;

// application settings file path
const char* FILE_PATH_SETTINGS = APP_DATA_PATH("settings.conf");

static bool load_setting(Nrf24Tool* context) {
    size_t file_size = 0;

    // set defaults settings
    context->settings = &nrf24Tool_settings;
    memcpy(context->settings->sniff_settings, sniff_defaults, sizeof(sniff_defaults));
    memcpy(context->settings->badmouse_settings, badmouse_defaults, sizeof(badmouse_defaults));

    context->storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = file_stream_alloc(context->storage);

    if(file_stream_open(stream, FILE_PATH_SETTINGS, FSAM_READ, FSOM_OPEN_EXISTING)) {
        file_size = stream_size(stream);
        stream_seek(stream, 0, StreamOffsetFromStart);

        if(file_size > 0) {
            FuriString* line = furi_string_alloc();
            // Lire le fichier ligne par ligne
            while(stream_read_line(stream, line)) {
                // skip line if comment
                char first_char = furi_string_get_char(line, 0);
                if(first_char == '#' || first_char == ' ') continue;

                // find parameter name + value
                size_t equal_pos = furi_string_search_char(line, '=');
                if(equal_pos == FURI_STRING_FAILURE) continue;
                FuriString* key = furi_string_alloc_set(line);
                FuriString* value = furi_string_alloc_set(line);
                furi_string_left(key, equal_pos);
                furi_string_right(value, equal_pos + 1);

                // find parameter name in settings map
                for(uint8_t i = 0; i < SETTINGS_QTY; i++) {
                    if(furi_string_cmp_str(key, settings_map[i].key) == 0) {
                        // Fetch the string value
                        const char* value_str = furi_string_get_cstr(value);

                        // Handle conversion for numeric types only
                        int value_int = 0;
                        if(settings_map[i].type != SETTING_TYPE_STRING)
                            value_int =
                                atoi(value_str); // Only convert to int if it's not a string

                        switch(settings_map[i].type) {
                        case SETTING_TYPE_UINT8:
                            *((uint8_t*)settings_map[i].target) = (uint8_t)value_int;
                            break;
                        case SETTING_TYPE_UINT16:
                            *((uint16_t*)settings_map[i].target) = (uint16_t)value_int;
                            break;
                        case SETTING_TYPE_UINT32:
                            *((uint32_t*)settings_map[i].target) = (uint32_t)value_int;
                            break;
                        case SETTING_TYPE_BOOL:
                            if(value_int == 1)
                                *((bool*)settings_map[i].target) = true;
                            else
                                *((bool*)settings_map[i].target) = false;
                            break;
                        case SETTING_TYPE_DATA_RATE:
                            *((nrf24_data_rate*)settings_map[i].target) = (uint8_t)value_int;
                            break;
                        case SETTING_TYPE_TX_POWER:
                            *((nrf24_tx_power*)settings_map[i].target) = (uint8_t)value_int;
                            break;
                        case SETTING_TYPE_ADDR_WIDTH:
                            *((nrf24_addr_width*)settings_map[i].target) = (uint8_t)value_int;
                            break;
                        case SETTING_TYPE_STRING:
                            strncpy(
                                (char*)settings_map[i].target, value_str, furi_string_size(value));
                            break;
                        }
                    }
                }
                furi_string_free(key);
                furi_string_free(value);
            }
            furi_string_free(line);
        }
    }
    file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);

    return true;
}

static bool save_setting(Nrf24Tool* context) {
    //size_t file_size = 0;
    bool ret = false;

    context->storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = file_stream_alloc(context->storage);

    if(file_stream_open(stream, FILE_PATH_SETTINGS, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        for(uint8_t i = 0; i < SETTINGS_QTY; i++) {
            switch(settings_map[i].type) {
            case SETTING_TYPE_UINT8:
                stream_write_format(
                    stream, "%s=%u\n", settings_map[i].key, *((uint8_t*)settings_map[i].target));
                continue;
            case SETTING_TYPE_UINT16:
                stream_write_format(
                    stream, "%s=%u\n", settings_map[i].key, *((uint16_t*)settings_map[i].target));
                continue;
            case SETTING_TYPE_UINT32:
                stream_write_format(
                    stream, "%s=%lu\n", settings_map[i].key, *((uint32_t*)settings_map[i].target));
                continue;
            case SETTING_TYPE_BOOL:
                if(*(bool*)settings_map[i].target == true)
                    stream_write_format(stream, "%s=%u\n", settings_map[i].key, 1);
                else
                    stream_write_format(stream, "%s=%u\n", settings_map[i].key, 0);
                continue;
            case SETTING_TYPE_DATA_RATE:
            case SETTING_TYPE_TX_POWER:
            case SETTING_TYPE_ADDR_WIDTH:
                stream_write_format(
                    stream, "%s=%u\n", settings_map[i].key, *((uint8_t*)settings_map[i].target));
                continue;
            case SETTING_TYPE_STRING:
                stream_write_format(
                    stream, "%s=%s\n", settings_map[i].key, (char*)settings_map[i].target);
                continue;
            }
        }
        ret = true;
    } else {
        FURI_LOG_E(LOG_TAG, "Error saving settings to file");
    }

    file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);

    return ret;
}

/* Allocate the memory and initialise the variables */
static Nrf24Tool* nrf24Tool_alloc(void) {
    Nrf24Tool* app = malloc(sizeof(Nrf24Tool));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    app->app_menu = submenu_alloc();
    submenu_set_header(app->app_menu, "Select Tool");
    submenu_add_item(app->app_menu, "Reception", MENU_RX, app_menu_enter_callback, app);
    submenu_add_item(app->app_menu, "Emission", MENU_TX, app_menu_enter_callback, app);
    submenu_add_item(app->app_menu, "Sniffer", MENU_SNIFFER, app_menu_enter_callback, app);
    submenu_add_item(app->app_menu, "Bad Mouse", MENU_BADMOUSE, app_menu_enter_callback, app);
    view_set_previous_callback(submenu_get_view(app->app_menu), app_menu_exit_callback);
    view_dispatcher_add_view(app->view_dispatcher, VIEW_MENU, submenu_get_view(app->app_menu));
    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_MENU);

    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    app->sniff_thread = furi_thread_alloc_ex("Sniff", 1024, nrf24_sniff, app);
    app->badmouse_thread = furi_thread_alloc_ex("BadMouse", 2048, nrf24_badmouse, app);

    app->notification = furi_record_open(RECORD_NOTIFICATION);

    return app;
}

/* Release the unused resources and deallocate memory */
static void nrf24Tool_free(Nrf24Tool* app) {
    view_dispatcher_remove_view(app->view_dispatcher, VIEW_MENU);
    submenu_free(app->app_menu);
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_NRF24_NOT_CONNECTED);
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_RX_SETTINGS);
    //variable_item_list_free(app->rx_settings);
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_RX_RUN);
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_TX_SETTINGS);
    //variable_item_list_free(app->tx_settings);
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_TX_RUN);
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_SNIFF_SETTINGS);
    
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_BM_RUN);
    //view_dispatcher_remove_view(app->view_dispatcher, VIEW_BM_NO_CHANNEL);

    view_dispatcher_free(app->view_dispatcher);

    furi_message_queue_free(app->event_queue);

    furi_thread_free(app->sniff_thread);
    furi_thread_free(app->badmouse_thread);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);

    free(app);
}

/* Starts the reader thread and handles the input */
static void nrf24Tool_run(Nrf24Tool* app) {
    // Init NRF24 communication
    nrf24_init();

    // load application settings
    if(!load_setting(app)) FURI_LOG_E(LOG_TAG, "Unable to load application settings !");

    // Alloc tools
    sniff_alloc(app);
    badmouse_alloc(app);

    // Program main
    view_dispatcher_run(app->view_dispatcher);

    // Free tools
    sniff_free(app);
    badmouse_free(app);

    // Deinitialize the nRF24 module
    nrf24_deinit();
}

int32_t nrf24tool_app(void* p) {
    UNUSED(p);

    //nrf24_sniff(3000);
    nrf24Tool_app = nrf24Tool_alloc();

    // run program
    nrf24Tool_run(nrf24Tool_app);

    // save settings before exit
    save_setting(nrf24Tool_app);

    // free program
    nrf24Tool_free(nrf24Tool_app);

    return 0;
}
