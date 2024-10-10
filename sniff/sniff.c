#include "sniff.h"
#include "../helper.h"
#include "../libnrf24/nrf24.h"
#include "notification/notification_messages.h"
#include "stream/file_stream.h"

//#define LOG_TAG "NRF24_sniff"

uint8_t preamble[] = {0xAA, 0x00};

uint8_t candidates[MAX_ADDRS][5] = {0}; // last 100 sniffed addresses
uint32_t counts[MAX_ADDRS];
uint8_t confirmed[MAX_CONFIRMED][5] = {0}; // first 32 confirmed addresses
uint8_t confirmed_idx = 0;
const char* FILE_PATH_ADDR = APP_DATA_PATH("addresses.txt");
uint32_t total_candidates = 0;
uint32_t candidate_idx = 0;
char top_address[HEX_MAC_LEN];
nrf24_data_rate target_rate = DATA_RATE_2MBPS;

SniffStatus sniff_status = {
    .current_channel = 0,
    .addr_find_count = 0,
};

Setting sniff_defaults[] = {
    {.name = "Min. Channel",
     .type = SETTING_TYPE_UINT8,
     .value.u8 = DEFAULT_MIN_CHANNEL,
     .min = DEFAULT_MIN_CHANNEL,
     .max = DEFAULT_MAX_CHANNEL,
     .step = 1},
    {.name = "Max. Channel",
     .type = SETTING_TYPE_UINT8,
     .value.u8 = DEFAULT_MAX_CHANNEL,
     .min = DEFAULT_MIN_CHANNEL,
     .max = DEFAULT_MAX_CHANNEL,
     .step = 1},
    {.name = "Scan Time (µs)",
     .type = SETTING_TYPE_UINT16,
     .value.u16 = DEFAULT_SCANTIME,
     .min = 500,
     .max = 5000,
     .step = 500},
    {.name = "Data Rate",
     .type = SETTING_TYPE_DATA_RATE,
     .value.d_r = DATA_RATE_2MBPS,
     .min = DATA_RATE_1MBPS,
     .max = DATA_RATE_250KBPS,
     .step = 1},
     {.name = "Rx Power Detector",
     .type = SETTING_TYPE_BOOL,
     .value.b = true,
     .min = 0,
     .max = 1,
     .step = 1}
};

NRF24L01_Config sniff = {
    .channel = DEFAULT_MIN_CHANNEL,
    .data_rate = DATA_RATE_2MBPS,
    .tx_power = TX_POWER_0DBM,
    .crc_length = 0,
    .mac_len = ADDR_WIDTH_2_BYTES,
    .arc = 15,
    .ard = 250,
    .auto_ack = {false, false, false, false, false, false},
    .dynamic_payload = {false, false, false, false, false, false},
    .ack_payload = false,
    .tx_no_ack = false,
    .tx_addr = NULL,
    .rx_addr = {preamble, NULL, NULL, NULL, NULL, NULL},
    .payload_size = {MAX_PAYLOAD_SIZE, 0, 0, 0, 0, 0}};

NRF24L01_Config find_channel_config = {
    .channel = 0,
    .data_rate = DATA_RATE_2MBPS,
    .tx_power = TX_POWER_0DBM,
    .crc_length = 2,
    .mac_len = ADDR_WIDTH_5_BYTES,
    .arc = 15,
    .ard = 500,
    .auto_ack = {true, false, false, false, false, false},
    .dynamic_payload = {true, false, false, false, false, false},
    .ack_payload = false,
    .tx_no_ack = false,
    .tx_addr = NULL,
    .rx_addr = {NULL, NULL, NULL, NULL, NULL, NULL},
    .payload_size = {0, 0, 0, 0, 0, 0}};

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
    for(int i = 0; i < MAX_CONFIRMED; i++) {
        if(memcmp(confirmed[i], addr, MAX_MAC_SIZE) == 0) return true;
    }
    return false;
}

static void alt_address(uint8_t* addr, uint8_t* altaddr) {
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

static bool validate_address(uint8_t* addr) {
    uint16_t bad[] = {0x5555, 0xAAAA, 0x0000, 0xFFFF};
    uint16_t* addr16 = (uint16_t*)addr;

    for(int i = 0; i < 4; i++) {
        if(addr16[0] == bad[i] || addr16[1] == bad[i]) {
            return false;
        }
    }
    return true;
}

static bool nrf24_sniff_address(uint8_t* address, bool rpd) {
    uint8_t packet[32] = {0};
    uint8_t packetsize;
    uint8_t rpd_value;

    if(nrf24_rxpacket(packet, &packetsize, true)) {
        nrf24_read_reg(REG_RPD, &rpd_value, 1);
        if((rpd_value & 0x01) || !rpd) {
            if(validate_address(packet)) {
                uint8_t j = MAX_MAC_SIZE - 1;
                for(uint8_t i = 0; i < MAX_MAC_SIZE; i++) {
                    address[i] = packet[j - i];
                }
                return true;
            }
        }
    }
    return false;
}

static uint8_t nrf24_find_channel(
    uint8_t* srcmac,
    uint8_t* dstmac,
    nrf24_addr_width maclen,
    nrf24_data_rate rate,
    uint8_t min_channel,
    uint8_t max_channel) {
    uint8_t ping_packet[] = {0x0f, 0x0f, 0x0f, 0x0f};
    uint8_t ch;

    find_channel_config.data_rate = rate;
    find_channel_config.rx_addr[0] = srcmac;
    find_channel_config.tx_addr = dstmac;
    find_channel_config.mac_len = maclen;
    find_channel_config.channel = min_channel;

    nrf24_configure(&find_channel_config);

    for(ch = min_channel; ch <= max_channel; ch++) {
        nrf24_set_chan(ch);
        if(nrf24_txpacket(ping_packet, FIND_CHANNEL_PAYLOAD_SIZE, find_channel_config.tx_no_ack))
            return ch;
    }

    return max_channel + 1; // Échec
}

static void wrap_up(Nrf24Tool* context) {
    uint8_t ch;
    uint8_t addr[5];
    uint8_t altaddr[5];
    char trying[12];
    int idx;
    Setting* setting = context->settings->sniff_settings;
    uint8_t min_channel = setting[SNIFF_SETTING_MIN_CHANNEL].value.u8;
    uint8_t max_channel = setting[SNIFF_SETTING_MAX_CHANNEL].value.u8;

    nrf24_set_mode(MODE_IDLE);

    while(true) {
        idx = get_highest_idx();
        if(counts[idx] < COUNT_THRESHOLD) break;

        sniff_status.current_channel = 0;
        counts[idx] = 0;
        memcpy(addr, candidates[idx], 5);
        hexlify(addr, 5, trying);
        FURI_LOG_I(LOG_TAG, "trying address %s", trying);
        memcpy(sniff_status.tested_addr, trying, HEX_MAC_LEN);
        ch = nrf24_find_channel(
            addr, addr, ADDR_WIDTH_5_BYTES, target_rate, min_channel, max_channel);
        FURI_LOG_I(LOG_TAG, "find_channel returned %d", (int)ch);
        if(ch > max_channel) {
            alt_address(addr, altaddr);
            if(previously_confirmed(altaddr)) continue;
            hexlify(altaddr, 5, trying);
            FURI_LOG_I(LOG_TAG, "trying alternate address %s", trying);
            memcpy(sniff_status.tested_addr, trying, HEX_MAC_LEN);
            ch = nrf24_find_channel(
                altaddr, altaddr, ADDR_WIDTH_5_BYTES, target_rate, min_channel, max_channel);
            FURI_LOG_I(LOG_TAG, "find_channel returned %d", (int)ch);
            memcpy(addr, altaddr, 5);
        }

        if(ch <= max_channel) {
            hexlify(addr, 5, top_address);
            FURI_LOG_I(LOG_TAG, "Address found ! : %s", top_address);
            if(confirmed_idx < MAX_CONFIRMED) {
                memcpy(confirmed[confirmed_idx++], addr, ADDR_WIDTH_5_BYTES);
                stream_write_format(context->stream, "%s\n", top_address);
                notification_message(context->notification, &sequence_double_vibro);
            }
            sniff_status.addr_find_count = confirmed_idx;
            sniff_status.addr_new_count++;
        }
    }
}

static bool load_file(Nrf24Tool* context)
{
    size_t file_size = 0;

    context->storage = furi_record_open(RECORD_STORAGE);
    context->stream = file_stream_alloc(context->storage);

    if(file_stream_open(context->stream, FILE_PATH_ADDR, FSAM_READ_WRITE, FSOM_OPEN_ALWAYS)) {
        file_size = stream_size(context->stream);

        // file empty
        if (file_size == 0)
        {
            confirmed_idx = 0;
        }
        else {
            FuriString* line = furi_string_alloc();
            stream_rewind(context->stream);
            while(stream_read_line(context->stream, line)) {
                // copy previously find adresse
                size_t line_lenght = furi_string_size(line);
                if(line_lenght > HEX_MAC_LEN) continue;
                if(confirmed_idx < MAX_CONFIRMED) {
                    uint8_t addr[MAX_MAC_SIZE];
                    unhexlify(furi_string_get_cstr(line), (line_lenght - 1) / 2, addr);
                    if(!previously_confirmed(addr)) memcpy(confirmed[confirmed_idx++], addr, MAX_MAC_SIZE);
                    else { // delete duplicate address in file
                        stream_seek(context->stream, line_lenght * (-1), StreamOffsetFromCurrent);
                        stream_delete(context->stream, line_lenght);
                    }
                } else {
                    FURI_LOG_E(LOG_TAG, "Sniffing : confirmed buffer overflow!");
                    return false;
                }
            }
            furi_string_free(line);
        }
        sniff_status.addr_find_count = confirmed_idx;
        sniff_status.addr_new_count = 0;
        return true;
    }
    return false;
}

int32_t nrf24_sniff(void* ctx) {
    Nrf24Tool* context = (Nrf24Tool*)ctx;

    uint8_t address[MAX_MAC_SIZE];
    uint32_t start = 0;
    Setting* setting = context->settings->sniff_settings;
    uint8_t min_channel = setting[SNIFF_SETTING_MIN_CHANNEL].value.u8;
    uint8_t max_channel = setting[SNIFF_SETTING_MAX_CHANNEL].value.u8;
    uint8_t target_channel = min_channel;

    memcpy(sniff_status.tested_addr, EMPTY_HEX, HEX_MAC_LEN);
    memset(candidates, 0, sizeof(candidates));
    memset(counts, 0, sizeof(counts));
    memset(confirmed, 0, sizeof(confirmed));
    confirmed_idx = 0;
    total_candidates = 0;
    candidate_idx = 0;

    if(!load_file(context))
    {
        FURI_LOG_E(LOG_TAG, "Sniffing : error opening adresses file !");
        context->tool_running = false;
        file_stream_close(context->stream);
        stream_free(context->stream);
        furi_record_close(RECORD_STORAGE);
        return 1;
    }

    sniff.channel = target_channel;
    sniff.data_rate = context->settings->sniff_settings[SNIFF_SETTING_DATA_RATE].value.d_r;
    nrf24_configure(&sniff);
    sniff_status.current_channel = target_channel;

    start = furi_get_tick();

    while(context->tool_running) {
        if(nrf24_sniff_address(address, setting[SNIFF_SETTING_RPD].value.b)) {
            int idx;
            uint8_t* top_addr;
            if(!previously_confirmed(address)) {
                idx = get_addr_index(address, 5);
                hexlify(address, 5, top_address);
                FURI_LOG_I(LOG_TAG, "address finded : %s", top_address);
                notification_message(context->notification, &sequence_blink_blue_10);
                if(idx == -1)
                    insert_addr(address, 5);
                else {
                    counts[idx]++;
                    FURI_LOG_I(LOG_TAG, "address count : %ld", counts[idx]);
                }

                top_addr = candidates[get_highest_idx()];
                hexlify(top_addr, 5, top_address);
            }
        }

        if(furi_get_tick() - start >= setting[SNIFF_SETTING_SCAN_TIME].value.u16) {
            target_channel++;
            if(target_channel > max_channel) target_channel = min_channel;

            wrap_up(context);
            sniff.channel = target_channel;
            memcpy(sniff_status.tested_addr, EMPTY_HEX, HEX_MAC_LEN);
            nrf24_configure(&sniff);
            nrf24_set_mode(MODE_RX);
            sniff_status.current_channel = target_channel;
            FURI_LOG_I(LOG_TAG, "new channel : %d", nrf24_get_chan());
            start = furi_get_tick();
        }

        // delay for main thread
        furi_delay_ms(100);
    }
    // close addresses file
    file_stream_close(context->stream);
    stream_free(context->stream);
    furi_record_close(RECORD_STORAGE);

    return 0;
}
