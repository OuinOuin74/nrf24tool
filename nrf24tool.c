#include <stream/stream.h>
#include "stream/file_stream.h"

#include "helper.h"
#include "nrf24tool.h"
#include "libnrf24/nrf24.h"
#include "sniff/sniff.h"
#include "settings.h"

// Main app variable
Nrf24Tool* nrf24Tool_app = NULL;

// application settings file path
const char* FILE_PATH_SETTINGS = APP_DATA_PATH("settings.conf");

static bool load_setting(Nrf24Tool* context)
{
    size_t file_size = 0;

    // set defaults settings
    context->settings = &nrf24Tool_settings;
    context->settings->sniff_settings = sniff_defaults;

    Stream* stream = file_stream_alloc(context->storage);

    if(file_stream_open(stream, FILE_PATH_SETTINGS, FSAM_READ_WRITE, FSOM_OPEN_APPEND))
    {
        file_size = stream_size(stream);
        stream_seek(stream, 0, StreamOffsetFromStart);
        
        if(file_size > 0)
        {
            FuriString* line = furi_string_alloc();
            // Lire le fichier ligne par ligne
            while(stream_read_line(stream, line)) {
                // Afficher la ligne lue
                printf("Ligne lue : %s\n", furi_string_get_cstr(line));
                // Ici, `stream_get_line()` déplace déjà le curseur automatiquement
            }
            furi_string_free(line);
        }
        

    }
    stream_free(stream);

    return true;
}

/* Draw the GUI of the application. The screen is completely redrawn during each call. */
static void draw_callback(Canvas* canvas, void* ctx) {
    Nrf24Tool* context = ctx;

    switch (context->currentMode) {
        case MODE_SNIFF_SETTINGS:
            sniff_draw(canvas, context);
            break;

        default:
            break;
    }
}

/* This function is called from the GUI thread. All it does is put the event
   into the application's queue so it can be processed later. */
static void input_callback(InputEvent* event, void* ctx) {
    Nrf24Tool* context = ctx;
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

    if (!nrf24_check_connected()) context->currentMode = MODE_RF24_DISCONNECTED;
    
    context->currentMode = MODE_SNIFF_SETTINGS;

    // load application settings
    if(!load_setting(context))
        FURI_LOG_E(LOG_TAG, "Unable to load application settings !");

    /* Endless main program loop */
    for(bool is_running = true; is_running;) {
        InputEvent event;
        /* Wait for an input event. Input events come from the GUI thread via a callback. */
        const FuriStatus status =
            furi_message_queue_get(context->event_queue, &event, FuriWaitForever);

        /* This application is only interested in short button presses. */
        if((status != FuriStatusOk) || (event.type != InputTypeShort && event.type != InputTypeRepeat)) {
            continue;
        }

        /* When the user presses the "Back" button, break the loop and exit the application. */
        if(event.key == InputKeyBack) {
            is_running = false;
            FURI_LOG_I(LOG_TAG, "back key presse");
        }
    }

    // Deinitialize the nRF24 module
    nrf24_deinit();
}

int32_t nrf24tool_app(void* p)
{
    UNUSED(p);

    //nrf24_sniff(3000);
    nrf24Tool_app = nrf24Tool_alloc();

    // run program
    nrf24Tool_run(nrf24Tool_app);

    // free program
    nrf24Tool_free(nrf24Tool_app);
    
    return 0;
}
