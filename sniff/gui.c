#include "sniff.h"
#include "../helper.h"

static uint8_t confirmed_idx_draw = 0;
static const uint8_t start_x = 2;
static const uint8_t start_y = 14;
static const uint8_t step = 13;
VariableItem* sniff_item[SNIFF_SETTING_COUNT];

static void run_draw_callback(Canvas* canvas, void* model) {
    Nrf24Tool* app = (Nrf24Tool*)model;

    //const size_t middle_x = canvas_width(canvas) / 2U;
    canvas_set_color(canvas, ColorBlack);

    // draw title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Mode : Sniffing");
    canvas_draw_line(canvas, 0, 10, canvas_width(canvas), 10);

    // draw status
    canvas_set_font(canvas, FontSecondary);
    // application status
    uint8_t line_num = 0;
    if(app->tool_running)
        canvas_draw_str_aligned(
            canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, "Status : RUN");
    else
        canvas_draw_str_aligned(
            canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, "Status : IDLE");
    // current channel
    line_num++;
    char status_str[30];
    if(sniff_status.current_channel != 0)
        snprintf(
            status_str, sizeof(status_str), "Scanning channel : %u", sniff_status.current_channel);
    else
        snprintf(status_str, sizeof(status_str), "Testing address : %s", sniff_status.tested_addr);
    canvas_draw_str_aligned(
        canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, status_str);
    // find address
    line_num++;
    if(confirmed_idx == 0) {
        strcpy(status_str, "Address found : NONE");
        confirmed_idx_draw = 0;
    } else {
        char hex_addr[11];
        hexlify(confirmed[confirmed_idx_draw], MAX_MAC_SIZE, hex_addr);
        snprintf(status_str, sizeof(status_str), "Address found : %s", hex_addr);
    }
    canvas_draw_str_aligned(
        canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, status_str);
    line_num++;
    // counters
    snprintf(
        status_str,
        sizeof(status_str),
        "Found : %u    |    New : %u",
        sniff_status.addr_find_count,
        sniff_status.addr_new_count);
    canvas_draw_str_aligned(
        canvas, start_x, start_y + line_num * step, AlignLeft, AlignTop, status_str);
}

static void input_run(InputEvent* event, Nrf24Tool* context) {
    switch(event->key) {
    case InputKeyLeft:
        if(confirmed_idx_draw > 0) confirmed_idx_draw--;
        break;

    case InputKeyRight:
        if(confirmed_idx_draw < (confirmed_idx - 1)) confirmed_idx_draw++;
        break;

    case InputKeyOk:
        if(!context->tool_running) {
            context->tool_running = true;
            if(furi_thread_get_state(context->sniff_thread) == FuriThreadStateStopped)
                furi_thread_start(context->sniff_thread);
        }
        break;

    case InputKeyBack:
        if(context->tool_running)
            context->tool_running = false;
        else {
            context->currentMode = MODE_SNIFF_SETTINGS;
        }
        furi_thread_join(context->sniff_thread);
        break;

    default:
        break;
    }
    if(confirmed_idx_draw > (confirmed_idx - 1)) confirmed_idx_draw = (confirmed_idx - 1);
}

static void item_change_callback(VariableItem* item) {
    Nrf24Tool* app = (Nrf24Tool*)variable_item_get_context(item);
    
    uint8_t current_setting = variable_item_list_get_selected_item_index(app->sniff_settings);
    uint8_t current_index = variable_item_get_current_value_index(item);

    if(current_setting >= SNIFF_SETTING_COUNT) return;
    Setting* setting = &app->settings->sniff_settings[current_setting];
    set_setting_value(setting, (current_index * setting->step) + setting->min);
    
    char buffer[20];
    setting_value_to_string(app->settings->sniff_settings[current_setting], buffer, sizeof(buffer));
    variable_item_set_current_value_text(item, buffer);
}

static void settings_enter_callback(void* context, uint32_t index) {
    UNUSED(index);
    Nrf24Tool* app = (Nrf24Tool*)context;

    app->currentMode = MODE_SNIFF_RUN;
    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_SNIFF_RUN);
}

static uint32_t settings_exit_callback(void* context) {
    Nrf24Tool* app = (Nrf24Tool*)context;

    app->currentMode = MODE_MENU;
    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_MENU);
    return 0;
}

void sniff_alloc(Nrf24Tool* app) {
    app->sniff_settings = variable_item_list_alloc();

    variable_item_list_set_header(app->sniff_settings, "Mode : Sniffing");

    for(uint8_t i = 0; i < SNIFF_SETTING_COUNT; i++) {
        sniff_item[i] = variable_item_list_add(
            app->sniff_settings,
            app->settings->sniff_settings[i].name,
            MAX_SETTINGS(app->settings->sniff_settings[i]),
            item_change_callback,
            app);
    }

    variable_item_list_set_enter_callback(app->sniff_settings, settings_enter_callback, app);
    view_set_previous_callback(variable_item_list_get_view(app->sniff_settings), settings_exit_callback);
    view_dispatcher_add_view(app->view_dispatcher, VIEW_SNIFF_SETTINGS, variable_item_list_get_view(app->sniff_settings));

    app->sniff_run = view_alloc();
    
}

void sniff_free(Nrf24Tool* app) {
    view_dispatcher_remove_view(app->view_dispatcher, VIEW_SNIFF_RUN);
    view_dispatcher_remove_view(app->view_dispatcher, VIEW_BM_SETTINGS);
    variable_item_list_free(app->badmouse_settings);

    view_free(app->sniff_run);
}
