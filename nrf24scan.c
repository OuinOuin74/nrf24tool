#include <furi.h>

/* generated by fbt from .png files in images folder */
#include <nrf24scan_icons.h>
#include "libnrf24/nrf24.h"

#define LOG_TAG "NRF24SCAN"
#define MAX_CHANNEL	125
#define MAX_ADDR	6

int32_t nrf24scan_app(void* p)
{
    UNUSED(p);

    FURI_LOG_I(LOG_TAG,"Init nrf24");
    nrf24_init();
    FURI_LOG_I(LOG_TAG, "NFR24 status : %d", nrf24_status());
    nrf24_set_chan(11);
    FURI_LOG_I(LOG_TAG, "NFR24 current channel : %d", nrf24_get_chan());

    nrf24_set_rx_mode();
    FURI_LOG_I(LOG_TAG, "NFR24 status : %d", nrf24_status());

    nrf24_deinit();

    return 0;
}
