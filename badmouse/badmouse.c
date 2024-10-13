#include "badmouse.h"
#include "../helper.h"
#include "../libnrf24/nrf24.h"
#include "notification/notification_messages.h"


NRF24L01_Config bm_config = {
    .channel = LOGITECH_MIN_CHANNEL,
    .data_rate = DATA_RATE_2MBPS,
    .tx_power = TX_POWER_0DBM,
    .crc_length = 2,
    .mac_len = ADDR_WIDTH_5_BYTES,
    .arc = 15,
    .ard = 4000,
    .auto_ack = {true, false, false, false, false, false},
    .dynamic_payload = {true, false, false, false, false, false},
    .ack_payload = false,
    .tx_no_ack = false,
    .tx_addr = NULL,
    .rx_addr = {NULL, NULL, NULL, NULL, NULL, NULL},
    .payload_size = {MAX_PAYLOAD_SIZE, 0, 0, 0, 0, 0}};

Setting badmouse_defaults[] = {
    {.name = "ADDR Index", .type = SETTING_TYPE_UINT8, .value.u8 = 0, .min = 0, .max = 0, .step = 1},
    {.name = "Keyboard Layout",
     .type = SETTING_TYPE_UINT8,
     .value.u8 = 0,
     .min = 0,
     .max = 0,
     .step = 1},
    {.name = "Data Rate",
     .type = SETTING_TYPE_DATA_RATE,
     .value.d_r = DATA_RATE_2MBPS,
     .min = DATA_RATE_1MBPS,
     .max = DATA_RATE_250KBPS,
     .step = 1},
    {.name = "Tx Power",
     .type = SETTING_TYPE_TX_POWER,
     .value.t_p = TX_POWER_0DBM,
     .min = TX_POWER_M18DBM,
     .max = TX_POWER_0DBM,
     .step = 1}};

BadMouse bm_instance = {
    .config = &bm_config
};


int32_t nrf24_badmouse(void* ctx) {
    Nrf24Tool* context = (Nrf24Tool*)ctx;
    Setting* settings = context->settings->badmouse_settings;
    
    // Set NRF24 settings
    bm_config.data_rate = settings[BADMOUSE_SETTING_DATA_RATE].value.d_r;
    bm_config.tx_power = settings[BADMOUSE_SETTING_TX_POWER].value.t_p;
    static uint8_t tx_addr[MAX_MAC_SIZE];
    unhexlify(addr_list[settings[BADMOUSE_SETTING_ADDR_INDEX].value.u8], MAX_MAC_SIZE, tx_addr);
    bm_config.tx_addr = tx_addr;
    bm_config.rx_addr[0] = tx_addr;
    nrf24_configure(&bm_config);
    
    // test if a channel is founded
    if(find_channel(&bm_config) > LOGITECH_MAX_CHANNEL) {
        // display error message
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_text(message, "Channel not found !", 10, 20, AlignCenter, AlignCenter);
        dialog_message_set_header(message, "ERROR", 10, 10, AlignCenter, AlignCenter);
        dialog_message_set_buttons(message, "Return", NULL, NULL);
        /* if(dialog_message_show(dialogs, context->dialogs) == DialogMessageButtonBack) {
            context->tool_running = false;
            context->currentMode = MODE_BADMOUSE_SETTINGS;
            furi_record_close(RECORD_DIALOGS);
            return 1;
        } */
        FURI_LOG_I(LOG_TAG, "display dialog");
        while (1) {
            if(dialog_message_show(dialogs, message) == DialogMessageButtonBack)
                break;
            furi_delay_ms(100);
        }
        FURI_LOG_I(LOG_TAG, "dialog ends");
        context->tool_running = false;
        //context->currentMode = MODE_BADMOUSE_SETTINGS;
        dialog_message_free(message);
        furi_record_close(RECORD_DIALOGS);
        return 1;
    } 

    
    return 0;
}
