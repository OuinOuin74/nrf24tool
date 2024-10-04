#include <furi.h>

/* generated by fbt from .png files in images folder */
#include <nrf24tool_icons.h>
#include "nrf24tool.h"
#include "libnrf24/nrf24.h"

#define MAX_ADDRS            100
#define MAX_CONFIRMED        32
#define LOGITECH_MAX_CHANNEL 85
#define COUNT_THRESHOLD      2

uint8_t preamble[] = {0xAA, 0x00};

uint8_t candidates[MAX_ADDRS][5] = {0}; // last 100 sniffed addresses
uint32_t counts[MAX_ADDRS];
uint8_t confirmed[MAX_CONFIRMED][5] = {0}; // first 32 confirmed addresses
uint8_t confirmed_idx = 0;
uint32_t total_candidates = 0;
uint32_t candidate_idx = 0;
char top_address[12];
nrf24_data_rate target_rate = DATA_RATE_2MBPS; // rate can be either 8 (2Mbps) or 0 (1Mbps)

NRF24L01_Config sniff = {
    .channel = 0,
    .data_rate = DATA_RATE_1MBPS,
    .tx_power = TX_POWER_0DBM,
    .crc_length = 0,
    .mac_len = ADDR_WIDTH_2_BYTES,
    .arc = 0,
    .ard = 250,
    .auto_ack = false,
    .dynamic_payload = false,
    .ack_payload = false,
    .tx_no_ack = true,
    .enable_crc = false,
    .tx_addr = NULL,
    .rx_addr = preamble,
    .payload_size = MAX_PAYLOAD_SIZE
};

uint8_t run;

void input_callback(InputEvent* input_event, void* context) {
    UNUSED(context);
    if(input_event->type == InputTypePress) {
        switch(input_event->key) {
            case InputKeyUp:
                break;
            case InputKeyDown:
                break;
            case InputKeyLeft:
                break;
            case InputKeyRight:
                break;
            case InputKeyOk:
                break;
            case InputKeyBack:
                run = 0;
                break;
            case InputKeyMAX:
                break;
        }
    }
}

static void hexlify(uint8_t* in, uint8_t size, char* out) {
    memset(out, 0, size * 2);
    for(int i = 0; i < size; i++)
        snprintf(out + strlen(out), sizeof(out + strlen(out)), "%02X", in[i]);
}

static int get_addr_index(uint8_t* addr, uint8_t addr_size) {
    for(uint32_t i = 0; i < total_candidates; i++) {
        if(!memcmp(candidates[i], addr, addr_size)) return i;
    }

    return -1;
}

static int get_highest_idx() {
    uint32_t highest = 0;
    int highest_idx = 0;
    for(uint32_t i = 0; i < total_candidates; i++) {
        if(counts[i] > highest) {
            highest = counts[i];
            highest_idx = i;
        }
    }

    return highest_idx;
}

static void insert_addr(uint8_t* addr, uint8_t addr_size) {
    if(candidate_idx >= MAX_ADDRS) candidate_idx = 0;

    memcpy(candidates[candidate_idx], addr, addr_size);
    counts[candidate_idx] = 1;
    if(total_candidates < MAX_ADDRS) total_candidates++;
    candidate_idx++;
}

static bool previously_confirmed(uint8_t* addr) {
    bool found = false;
    for(int i = 0; i < MAX_CONFIRMED; i++) {
        if(!memcmp(confirmed[i], addr, 5)) {
            found = true;
            break;
        }
    }

    return found;
}

void alt_address(uint8_t* addr, uint8_t* altaddr) {
    uint8_t macmess_hi_b[4];
    uint32_t macmess_hi;
    uint8_t macmess_lo;
    uint8_t preserved;
    uint8_t tmpaddr[5];

    // swap bytes
    for(int i = 0; i < 5; i++)
        tmpaddr[i] = addr[4 - i];

    // get address into 32-bit and 8-bit variables
    memcpy(macmess_hi_b, tmpaddr, 4);
    macmess_lo = tmpaddr[4];

    macmess_hi = bytes_to_int32(macmess_hi_b, true);

    //preserve lowest bit from hi to shift to low
    preserved = macmess_hi & 1;
    macmess_hi >>= 1;
    macmess_lo >>= 1;
    macmess_lo = (preserved << 7) | macmess_lo;
    int32_to_bytes(macmess_hi, macmess_hi_b, true);
    memcpy(tmpaddr, macmess_hi_b, 4);
    tmpaddr[4] = macmess_lo;

    // swap bytes back
    for(int i = 0; i < 5; i++)
        altaddr[i] = tmpaddr[4 - i];
}

static void wrap_up() {
    uint8_t ch;
    uint8_t addr[5];
    uint8_t altaddr[5];
    char trying[12];
    int idx;

    nrf24_set_mode(MODE_IDLE);

    while(true) {
        idx = get_highest_idx();
        if(counts[idx] < COUNT_THRESHOLD) break;

        counts[idx] = 0;
        memcpy(addr, candidates[idx], 5);
        hexlify(addr, 5, trying);
        FURI_LOG_I(LOG_TAG, "trying address %s", trying);
        ch = nrf24_find_channel(addr, addr, ADDR_WIDTH_5_BYTES, target_rate, 2, LOGITECH_MAX_CHANNEL, false);
        FURI_LOG_I(LOG_TAG, "find_channel returned %d", (int)ch);
        if(ch > LOGITECH_MAX_CHANNEL) {
            alt_address(addr, altaddr);
            hexlify(altaddr, 5, trying);
            FURI_LOG_I(LOG_TAG, "trying alternate address %s", trying);
            ch = nrf24_find_channel(altaddr, altaddr, ADDR_WIDTH_5_BYTES, target_rate, 2, LOGITECH_MAX_CHANNEL, false);
            FURI_LOG_I(LOG_TAG, "find_channel returned %d", (int)ch);
            memcpy(addr, altaddr, 5);
        }

        if(ch <= LOGITECH_MAX_CHANNEL) {
            hexlify(addr, 5, top_address);
            FURI_LOG_I(LOG_TAG, "Address found ! : %s",top_address);
            if(confirmed_idx < MAX_CONFIRMED) memcpy(confirmed[confirmed_idx++], addr, 5);
            break;
        }
    }
}

int32_t nrf24tool_app(void* p)
{
    UNUSED(p);

    ViewPort* view_port = view_port_alloc();
    view_port_input_callback_set(view_port, input_callback, NULL);

    FURI_LOG_I(LOG_TAG,"Init nrf24");
    nrf24_init();
    FURI_LOG_I(LOG_TAG, "NFR24 status : %d", nrf24_status());
    //nrf24_set_chan(11);
    uint8_t target_channel = 0;
    sniff.channel = target_channel;
    nrf24_configure(&sniff);
    //nrf24_init_promisc_mode(target_channel, 8);
    FURI_LOG_I(LOG_TAG, "NFR24 current channel : %d", nrf24_get_chan());

    nrf24_set_mode(MODE_RX);
    FURI_LOG_I(LOG_TAG, "NFR24 status : %d", nrf24_status());
    

    uint8_t address[5] = {0};
    uint32_t start = 0;
    

    run = 1;

    while(run == 1) {
        if(nrf24_sniff_address(5, address)) {
            /* FURI_LOG_I(LOG_TAG, "Address Find ! :");
            for(uint8_t i = 0; i < 5; i++)
            {
                FURI_LOG_I(LOG_TAG, "%d", address[i]);
            } */
            int idx;
            uint8_t* top_addr;
            if(!previously_confirmed(address)) {
                idx = get_addr_index(address, 5);
                if(idx == -1)
                    insert_addr(address, 5);
                else
                    counts[idx]++;

                top_addr = candidates[get_highest_idx()];
                hexlify(top_addr, 5, top_address);
                FURI_LOG_I(LOG_TAG, "top address : %s", top_address);
            }
        }

        if(furi_get_tick() - start >= 2000) {
            target_channel++;
            if(target_channel > LOGITECH_MAX_CHANNEL) target_channel = 2;

            wrap_up();
            sniff.channel = target_channel;
            nrf24_configure(&sniff);
            nrf24_set_mode(MODE_RX);
            //nrf24_init_promisc_mode(target_channel, 8);
            FURI_LOG_I(LOG_TAG, "new channel : %d", nrf24_get_chan());
            start = furi_get_tick();
        }
    }

    // Deinitialize the nRF24 module
    nrf24_deinit();

    view_port_free(view_port);

    return 0;
}
