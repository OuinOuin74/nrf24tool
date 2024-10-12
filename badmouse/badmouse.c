#include "badmouse.h"
#include "../helper.h"
#include "../libnrf24/nrf24.h"
#include "notification/notification_messages.h"
#include "stream/file_stream.h"

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
    .payload_size = {MAX_PAYLOAD_SIZE, 0, 0, 0, 0, 0}
};

Setting badmouse_defaults[] = {
    {.name = "ADDR Index",
     .type = SETTING_TYPE_UINT8,
     .value.u8 = 0,
     .min = 0,
     .max = MAX_CONFIRMED_ADDR,
     .step = 1},
    {.name = "Keyboard Layout",
     .type = SETTING_TYPE_STRING,
     .value.str = DEFAULT_KB_LAYOUT,
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
     .step = 1}
};


int32_t nrf24_badmouse(void* ctx) {
    Nrf24Tool* context = (Nrf24Tool*)ctx;
    UNUSED(context);
    return 0;
};