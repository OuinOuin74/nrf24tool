#include <stream/stream.h>
#include "stream/file_stream.h"

#include "helper.h"
#include "nrf24tool.h"
#include "libnrf24/nrf24.h"
#include "sniff/sniff.h"
#include "settings.h"
#include "input.h"

// Main app variable
Nrf24Tool* nrf24Tool_app = NULL;

// application settings file path
const char* FILE_PATH_SETTINGS = APP_DATA_PATH("settings.conf");
const char* CURRENT_MODE = "app_mode";

static bool load_setting(Nrf24Tool* context) {
    size_t file_size = 0;

    // set defaults settings
    context->settings = &nrf24Tool_settings;
    memcpy(context->settings->sniff_settings, sniff_defaults, sizeof(sniff_defaults));

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

                // restore last mode
                if(furi_string_cmp_str(key, CURRENT_MODE) == 0) {
                    int mode_int = atoi(furi_string_get_cstr(value));
                    if(mode_int > 0) context->currentMode = mode_int;
                    continue;
                }

                // find parameter name in settings map
                for(uint8_t i = 0; i < SETTINGS_QTY; i++) {
                    if(furi_string_cmp_str(key, settings_map[i].key) == 0) {
                        int value_int = atoi(furi_string_get_cstr(value));
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
    FuriString* line = furi_string_alloc_set_str(CURRENT_MODE);

    if(file_stream_open(stream, FILE_PATH_SETTINGS, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        // save current mode
        furi_string_push_back(line, '=');
        furi_string_cat_printf(line, "%u", context->currentMode);
        stream_write_format(stream, "%s=%u\n", CURRENT_MODE, context->currentMode);
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
            }
        }
        ret = true;
    }
    else {
        FURI_LOG_E(LOG_TAG, "Error saving settings to file");
    }

    furi_string_free(line);
    file_stream_close(stream);
    stream_free(stream);
    furi_record_close(RECORD_STORAGE);

    return ret;
}

/* Draw the GUI of the application. The screen is completely redrawn during each call. */
static void draw_callback(Canvas* canvas, void* ctx) {
    Nrf24Tool* context = ctx;

    switch(context->currentMode) {
    case MODE_SNIFF_SETTINGS:
    case MODE_SNIFF_RUN:
        sniff_draw(canvas, context);
        break;

    default:
        break;
    }
}

/* This function is called from the GUI thread. All it does is put the event
   into the application's queue so it can be processed later. */
static void input_callback(InputEvent* event, void* ctx) {
    Nrf24Tool* context = (Nrf24Tool*)ctx;
    furi_message_queue_put(context->event_queue, event, FuriWaitForever);
}

/* Allocate the memory and initialise the variables */
static Nrf24Tool* nrf24Tool_alloc(void) {
    Nrf24Tool* context = malloc(sizeof(Nrf24Tool));

    context->view_port = view_port_alloc();
    view_port_draw_callback_set(context->view_port, draw_callback, context);
    view_port_input_callback_set(context->view_port, input_callback, context);

    context->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    context->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(context->gui, context->view_port, GuiLayerFullscreen);
    view_port_enabled_set(context->view_port, true);

    return context;
}

/* Release the unused resources and deallocate memory */
static void nrf24Tool_free(Nrf24Tool* context) {
    view_port_enabled_set(context->view_port, false);
    gui_remove_view_port(context->gui, context->view_port);

    furi_message_queue_free(context->event_queue);
    view_port_free(context->view_port);

    furi_record_close(RECORD_GUI);
}

/* Starts the reader thread and handles the input */
static void nrf24Tool_run(Nrf24Tool* context) {
    // Init NRF24 communication
    nrf24_init();

    if(!nrf24_check_connected()) context->currentMode = MODE_RF24_DISCONNECTED;

    // load application settings
    if(!load_setting(context)) FURI_LOG_E(LOG_TAG, "Unable to load application settings !");

    /* Endless main program loop */
    context->app_running = true;
    while(context->app_running) {
        
        // handle input actions
        handleEvent(context);
    }

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
