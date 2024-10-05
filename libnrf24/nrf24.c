// Modified by vad7, 25.11.2022
//
#include "nrf24.h"
//#include <furi.h>
//#include <furi_hal.h>
#include <furi_hal_resources.h>
#include <string.h>

uint8_t EMPTY_MAC[] = {0, 0, 0, 0, 0};
const uint8_t FIND_CHANNEL_PAYLOAD_SIZE = 4;

NRF24L01_Config find_channel_config = {
    .channel = 0,
    .data_rate = DATA_RATE_2MBPS,
    .tx_power = TX_POWER_0DBM,
    .crc_length = 2,
    .mac_len = ADDR_WIDTH_5_BYTES,
    .arc = 15,
    .ard = 500,
    .auto_ack = true,
    .dynamic_payload = true,
    .ack_payload = false,
    .tx_no_ack = false,
    .tx_addr = NULL,
    .rx_addr = {NULL, NULL, NULL, NULL, NULL, NULL},
    .payload_size = FIND_CHANNEL_PAYLOAD_SIZE
};

char LOG_TAG[] = "libNRF24";

void nrf24_init() {
    furi_hal_spi_bus_handle_init(&nrf24_HANDLE);
    furi_hal_spi_acquire(&nrf24_HANDLE);
    furi_hal_gpio_init(nrf24_CE_PIN, GpioModeOutputPushPull, GpioPullUp, GpioSpeedVeryHigh);
    furi_hal_gpio_write(nrf24_CE_PIN, false);
}

void nrf24_deinit() {
    furi_hal_spi_release(&nrf24_HANDLE);
    furi_hal_spi_bus_handle_deinit(&nrf24_HANDLE);
    furi_hal_gpio_write(nrf24_CE_PIN, false);
    furi_hal_gpio_init(nrf24_CE_PIN, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void nrf24_spi_trx(uint8_t* tx, uint8_t* rx, uint8_t size) {
    furi_hal_gpio_write(nrf24_HANDLE.cs, false);
    furi_hal_spi_bus_trx(&nrf24_HANDLE, tx, rx, size, nrf24_TIMEOUT);
    furi_hal_gpio_write(nrf24_HANDLE.cs, true);
}

uint8_t nrf24_write_reg(uint8_t reg, uint8_t data) {
    uint8_t tx[2] = {W_REGISTER | (REGISTER_MASK & reg), data};
    uint8_t rx[2] = {0};
    nrf24_spi_trx(tx, rx, 2);
    //FURI_LOG_D("NRF_WR", " #%02X=%02X", reg, data);
    return rx[0];
}

uint8_t nrf24_write_buf_reg(uint8_t reg, uint8_t* data, uint8_t size) {
    uint8_t tx[size + 1];
    uint8_t rx[size + 1];
    memset(rx, 0, size + 1);
    tx[0] = W_REGISTER | (REGISTER_MASK & reg);
    memcpy(&tx[1], data, size);
    nrf24_spi_trx(tx, rx, size + 1);
    //FURI_LOG_D("NRF_WR", " #%02X(%02X)=0x%02X%02X%02X%02X%02X", reg, size, data[0], data[1], data[2], data[3], data[4] );
    return rx[0];
}

uint8_t nrf24_read_reg(uint8_t reg, uint8_t* data, uint8_t size) {
    uint8_t tx[size + 1];
    uint8_t rx[size + 1];
    memset(rx, 0, size + 1);
    tx[0] = R_REGISTER | (REGISTER_MASK & reg);
    memset(&tx[1], 0, size);
    nrf24_spi_trx(tx, rx, size + 1);
    memcpy(data, &rx[1], size);
    return rx[0];
}

uint8_t nrf24_flush_rx() {
    uint8_t tx[] = {FLUSH_RX};
    uint8_t rx[] = {0};
    nrf24_spi_trx(tx, rx, 1);
    return rx[0];
}

uint8_t nrf24_flush_tx() {
    uint8_t tx[] = {FLUSH_TX};
    uint8_t rx[] = {0};
    nrf24_spi_trx(tx, rx, 1);
    return rx[0];
}

nrf24_addr_width nrf24_get_maclen() {
    uint8_t maclen;
    nrf24_read_reg(REG_SETUP_AW, &maclen, 1);
    maclen &= 3;
    return (nrf24_addr_width)(maclen + 2);
}

uint8_t nrf24_set_maclen(nrf24_addr_width maclen) {
    if(maclen < MIN_MAC_SIZE || maclen > MAX_MAC_SIZE) return 0;
    uint8_t status = 0;
    status = nrf24_write_reg(REG_SETUP_AW, maclen - 2);
    return status;
}

uint8_t nrf24_status() {
    uint8_t status;
    uint8_t tx[] = {R_REGISTER | REG_STATUS};
    nrf24_spi_trx(tx, &status, 1);
    return status;
}

nrf24_data_rate nrf24_get_rate() {
    uint8_t rate = 0;
    nrf24_read_reg(REG_RF_SETUP, &rate, 1);
    rate &= 0x28;
    if(rate == 0x20) // 250kbps
        return DATA_RATE_250KBPS;
    else if(rate == 0x08) // 2Mbps
        return DATA_RATE_2MBPS;
    else // 1Mbps
        return DATA_RATE_1MBPS;
}

uint8_t nrf24_set_rate(nrf24_data_rate rate) {
    uint8_t r6 = 0;
    uint8_t status = 0;

    nrf24_read_reg(REG_RF_SETUP, &r6, 1); // RF_SETUP register
    r6 &= 0xd7; // Clear rate fields.

    if(rate == DATA_RATE_250KBPS) r6 |= 0x20;
    if(rate == DATA_RATE_2MBPS) r6 |= 0x08;

    status = nrf24_write_reg(REG_RF_SETUP, r6); // Write new rate.
    return status;
}

uint8_t nrf24_get_chan() {
    uint8_t channel = 0;
    nrf24_read_reg(REG_RF_CH, &channel, 1);
    return channel;
}

uint8_t nrf24_set_chan(uint8_t chan) {
    if(chan > MAX_CHANNEL) return 0;
    uint8_t status;
    status = nrf24_write_reg(REG_RF_CH, chan);
    return status;
}

nrf24_tx_power nrf24_get_txpower() {
    uint8_t txpower = 0;
    nrf24_read_reg(REG_RF_SETUP, &txpower, 1);
    txpower = (txpower >> 1) & 0x03;
    return (nrf24_tx_power)txpower;
}

uint8_t nrf24_set_txpower(nrf24_tx_power txpower) {
    uint8_t status = 0;
    uint8_t rf_setup = 0;
    nrf24_read_reg(REG_RF_SETUP, &rf_setup, 1);
    rf_setup &= ~(0x06); // Clear bits 2 and 1 (RF_PWR)
    rf_setup |= ((uint8_t)txpower << 1); // set new tx power bits
    status = nrf24_write_reg(REG_RF_SETUP, rf_setup);
    return status;
}

uint8_t nrf24_set_crc(uint8_t length) {
    // Vérification des bornes pour la longueur du CRC
    if (length > MAX_CRC_LENGTH) return 0;  // MAX_CRC_LENGTH est 2 (0 = désactiver CRC, 1 = 1 byte, 2 = 2 bytes)

    uint8_t status = 0;
    uint8_t config = 0;

    // Lire la configuration actuelle du registre CONFIG
    nrf24_read_reg(REG_CONFIG, &config, 1);

    // Désactiver le CRC si length == 0
    if (length == 0) {
        config &= ~(0x08);  // Désactiver le bit EN_CRC (bit 3)
    } else {
        // Activer le CRC en réglant le bit EN_CRC (bit 3)
        config |= 0x08;

        // Mettre à jour la longueur du CRC dans le bit 2 (CRC0)
        if (length == 2) {
            config |= 0x04;  // Activer le bit 2 pour 2 bytes
        } else {
            config &= ~(0x04);  // Désactiver le bit 2 pour 1 byte
        }
    }

    // Écrire la nouvelle configuration dans le registre CONFIG
    status = nrf24_write_reg(REG_CONFIG, config);

    return status;  // Retourner le statut de l'écriture
}

nrf24_addr_width nrf24_get_rx_mac(uint8_t* mac, uint8_t pipe) {
    if (pipe > MAX_PIPE) return 0;
    nrf24_addr_width size = nrf24_get_maclen();
    nrf24_read_reg(REG_RX_ADDR_P0 + pipe, mac, size);
    return size;
}

uint8_t nrf24_set_rx_mac(uint8_t* mac, nrf24_addr_width size, uint8_t pipe) {
    if (pipe > MAX_PIPE || size < MIN_MAC_SIZE || size > MAX_MAC_SIZE) return 0;

    uint8_t status = 0;
    uint8_t reg_en_rxaddr = 0;

    nrf24_read_reg(REG_EN_RXADDR, &reg_en_rxaddr, 1);

    if (mac == NULL) {
        reg_en_rxaddr &= ~(1 << pipe);
        nrf24_write_reg(REG_EN_RXADDR, reg_en_rxaddr);
        return 1;
    }

    if (nrf24_get_maclen() != size) nrf24_set_maclen(size);

    uint8_t reg_addr = REG_RX_ADDR_P0 + pipe;
    nrf24_write_buf_reg(reg_addr, mac, size);

    reg_en_rxaddr |= (1 << pipe);
    status = nrf24_write_reg(REG_EN_RXADDR, reg_en_rxaddr);

    return status;  // Pipe activé avec succès
}

nrf24_addr_width nrf24_get_tx_mac(uint8_t* mac) {
    uint8_t size = nrf24_get_maclen();
    nrf24_read_reg(REG_TX_ADDR, mac, size);
    return size;
}

uint8_t nrf24_set_tx_mac(uint8_t* mac, nrf24_addr_width size) {
    if (size < MIN_MAC_SIZE || size > MAX_MAC_SIZE) return 0;
    uint8_t status = 0;
    nrf24_set_maclen(size);
    nrf24_write_buf_reg(REG_TX_ADDR, EMPTY_MAC, MAX_MAC_SIZE);
    status = nrf24_write_buf_reg(REG_TX_ADDR, mac, size);
    return status;
}

uint8_t nrf24_get_packetlen(uint8_t pipe) {
    if(pipe > MAX_PIPE) return 0;
    uint8_t len = 0;
    nrf24_read_reg(RX_PW_P0 + pipe, &len, 1);
    return len;
}

uint8_t nrf24_set_packetlen(uint8_t len) {
    if(len > MAX_PAYLOAD_SIZE || len < MIN_PAYLOAD_SIZE) return 0;
    uint8_t status = 0;
    status = nrf24_write_reg(RX_PW_P0, len);
    return status;
}

uint8_t nrf24_set_arc_ard(uint8_t arc, uint16_t ard) {
    if(arc > MAX_ARC_SIZE) return 0;
    if(ard < MIN_ARD_SIZE || ard > MAX_ARD_SIZE || ard % MIN_ARD_SIZE != 0) return 0;
    uint8_t status = 0;
    uint8_t ard_reg = (ard / MIN_ARD_SIZE) - 1;
    uint8_t re_tr = (ard_reg << 4) | (arc & 0x0f);
    status = nrf24_write_reg(REG_SETUP_RETR, re_tr);
    return status;
}

uint8_t nrf24_rxpacket(uint8_t* packet, uint8_t* packetsize, bool full) {
    uint8_t status = 0;
    uint8_t size = 0;
    uint8_t tx_pl_wid[] = {R_RX_PL_WID, 0};
    uint8_t rx_pl_wid[] = {0, 0};
    uint8_t tx_cmd[33] = {0}; // 32 max payload size + 1 for command
    uint8_t tmp_packet[33] = {0};

    status = nrf24_status();

    if(status & RX_DR) {
        if(full)
            size = nrf24_get_packetlen(0);
        else {
            nrf24_spi_trx(tx_pl_wid, rx_pl_wid, 2);
            size = rx_pl_wid[1];
        }

        tx_cmd[0] = R_RX_PAYLOAD;
        nrf24_spi_trx(tx_cmd, tmp_packet, size + 1);
        nrf24_write_reg(REG_STATUS, RX_DR); // clear bit.
        memcpy(packet, &tmp_packet[1], size);
    } else if(status == 0) {
        nrf24_flush_rx();
        nrf24_write_reg(REG_STATUS, RX_DR); // clear bit.
    }

    if (size != 0) *packetsize = size;
    return status;
}

// Return 0 when error
bool nrf24_txpacket(uint8_t* payload, uint8_t size, bool no_ack) {
    uint8_t status = 0;
    
    if (size > MAX_PAYLOAD_SIZE) return false;

    uint8_t tx[MAX_PAYLOAD_SIZE + 1] = {0};
    uint8_t rx[MAX_PAYLOAD_SIZE + 1] = {0};

    tx[0] = no_ack ? W_TX_PAYLOAD_NOACK : W_TX_PAYLOAD;

    memcpy(&tx[1], payload, size);
    nrf24_spi_trx(tx, rx, size + 1);

    nrf24_set_mode(MODE_TX);

    while(!(status & (TX_DS | MAX_RT))) status = nrf24_status();

    if(status & MAX_RT) nrf24_flush_tx();

    nrf24_set_mode(MODE_IDLE);

    return (status & TX_DS) ? true : false;
}

uint8_t nrf24_set_mode(nrf24_mode mode) {
    uint8_t status = 0;
    uint8_t cfg = 0;
    nrf24_read_reg(REG_CONFIG, &cfg, 1);

    switch(mode) {
    case MODE_IDLE:
        cfg &= 0xfc; // clear bottom two bits to power down the radio
        status = nrf24_write_reg(REG_CONFIG, cfg);
        furi_hal_gpio_write(nrf24_CE_PIN, false);
        break;

    case MODE_STANDBY:
        cfg |= 0x02; // PWR_UP
        status = nrf24_write_reg(REG_CONFIG, cfg);
        furi_hal_gpio_write(nrf24_CE_PIN, true);
        //furi_delay_ms(2);
        furi_delay_us(200);
        break;

    case MODE_RX:
        furi_hal_gpio_write(nrf24_CE_PIN, false);
        nrf24_write_reg(REG_STATUS, RX_DR);
        cfg |= 0x03; // PWR_UP, and PRIM_RX
        status = nrf24_write_reg(REG_CONFIG, cfg);
        furi_hal_gpio_write(nrf24_CE_PIN, true);
        //furi_delay_ms(2);
        furi_delay_us(200);
        break;

    case MODE_TX:
        furi_hal_gpio_write(nrf24_CE_PIN, false);
        nrf24_write_reg(REG_STATUS, TX_DS | MAX_RT);
        cfg &= 0xfe; // disable PRIM_RX
        cfg |= 0x02; // PWR_UP
        status = nrf24_write_reg(REG_CONFIG, cfg);
        furi_hal_gpio_write(nrf24_CE_PIN, true);
        //furi_delay_ms(2);
        furi_delay_us(200);
        break;
    }

    return status;
}

void nrf24_configure(NRF24L01_Config* config) {
    uint8_t reg_feature;

    nrf24_set_mode(MODE_IDLE); // power down
    nrf24_write_reg(REG_CONFIG, 0x00); // Stop nRF
    nrf24_write_reg(REG_STATUS, 0x70); // clear interrupts
    nrf24_write_reg(REG_FEATURE, 0x00); // clear features register
    
    nrf24_set_chan(config->channel); // set channel
    nrf24_set_rate(config->data_rate); //set data rate
    nrf24_set_txpower(config->tx_power); // set transmit power

    nrf24_set_maclen(config->mac_len); // set adress width 2,3,4 or 5 bytes

    if(config->ard >= MIN_ARD_SIZE && config->ard <= MAX_ARD_SIZE)
        nrf24_set_arc_ard(
            config->arc, config->ard); // set Auto retransmit count and Retransmit delay

    if(config->auto_ack) {
        nrf24_write_reg(REG_EN_AA, 0x3F); // Enable Shockburst
        config->crc_length = 2; // enable CRC
    } else
        nrf24_write_reg(REG_EN_AA, 0x00); // Disable Shockburst

    if(config->dynamic_payload) {
        nrf24_write_reg(REG_FEATURE, 0x04); // enable dyn payload
        if (config->auto_ack)
            nrf24_write_reg(REG_DYNPD, 0x3F); // enable dynamic payload length on all pipes
        else
            nrf24_write_reg(REG_DYNPD, 0x00); // disable dynamic payload length on all pipes
    } else {
        nrf24_write_reg(REG_DYNPD, 0x00); // disable dynamic payload length on all pipes
    }

    if(config->ack_payload) {
        nrf24_write_reg(REG_FEATURE, 0x06); // enable dyn payload and ack
        if(config->ard < 500)
            nrf24_set_arc_ard(config->arc, 500); // set ARD to at least 500µs according datasheet
    }

    if(config->tx_no_ack) // enable "NO_ACK" flag on tx frames
    {
        nrf24_read_reg(REG_FEATURE, &reg_feature, 1);
        reg_feature |= 0x01;
        nrf24_write_reg(REG_FEATURE, reg_feature);
    }

    nrf24_flush_rx();
    nrf24_flush_tx();

    nrf24_set_crc(config->crc_length); // CRC lenght 1 or 2 bytes 0 -> disable

    if(config->tx_addr) nrf24_set_tx_mac(config->tx_addr, config->mac_len); // set tx adress

    for(uint8_t i = 0; i <= MAX_PIPE; i++)
    {
        if (config->auto_ack && i == 0) // set rx adress (pipe 0) == tx adress if Enhanced ShockBurst enabled
        {
            uint8_t tx_addr[MAX_MAC_SIZE];
            uint8_t size;
            size = nrf24_get_tx_mac(tx_addr); // get current tx adress
            nrf24_set_rx_mac(tx_addr, size, i);
            continue;
        }
        nrf24_set_rx_mac(config->rx_addr[i], config->mac_len, i); // set rx adress for pipe i
    }

    if (config->payload_size >= MIN_PAYLOAD_SIZE && config->payload_size <= MAX_PAYLOAD_SIZE)
        nrf24_set_packetlen(config->payload_size); // set fix payload size for pipe 0

    furi_delay_ms(200);
}

void nrf24_init_promisc_mode(uint8_t channel, uint8_t rate) {
    //uint8_t preamble[] = {0x55, 0x00}; // little endian
    uint8_t preamble[] = {0xAA, 0x00}; // little endian
    //uint8_t preamble[] = {0x00, 0x55}; // little endian
    //uint8_t preamble[] = {0x00, 0xAA}; // little endian
    nrf24_write_reg(REG_CONFIG, 0x00); // Stop nRF
    nrf24_write_reg(REG_STATUS, 0x70); // clear interrupts
    nrf24_write_reg(REG_DYNPD, 0x0); // disable shockburst
    nrf24_write_reg(REG_EN_AA, 0x00); // Disable Shockburst
    nrf24_write_reg(REG_FEATURE, 0x05); // disable payload-with-ack, enable noack
    nrf24_set_maclen(2); // shortest address
    nrf24_set_rx_mac(preamble, 2,0); // set src mac to preamble bits to catch everything
    nrf24_set_packetlen(32); // set max packet length
    nrf24_set_mode(MODE_IDLE);
    nrf24_flush_rx();
    nrf24_flush_tx();
    nrf24_write_reg(REG_RF_CH, channel);
    nrf24_write_reg(REG_RF_SETUP, rate);

    // prime for RX, no checksum
    nrf24_write_reg(REG_CONFIG, 0x03); // PWR_UP and PRIM_RX, disable AA and CRC
    furi_hal_gpio_write(nrf24_CE_PIN, true);
    furi_delay_ms(100);
}

void hexlify(uint8_t* in, uint8_t size, char* out) {
    memset(out, 0, size * 2);
    for(int i = 0; i < size; i++)
        snprintf(out + strlen(out), sizeof(out + strlen(out)), "%02X", in[i]);
}

uint64_t bytes_to_int64(uint8_t* bytes, uint8_t size, bool bigendian) {
    uint64_t ret = 0;
    for(int i = 0; i < size; i++) {
        if(bigendian)
            ret |= bytes[i] << ((size - 1 - i) * 8);
        else
            ret |= bytes[i] << (i * 8);
    }

    return ret;
}

void int64_to_bytes(uint64_t val, uint8_t* out, bool bigendian) {
    for(int i = 0; i < 8; i++) {
        if(bigendian)
            out[i] = (val >> ((7 - i) * 8)) & 0xff;
        else
            out[i] = (val >> (i * 8)) & 0xff;
    }
}

uint32_t bytes_to_int32(uint8_t* bytes, bool bigendian) {
    uint32_t ret = 0;
    for(int i = 0; i < 4; i++) {
        if(bigendian)
            ret |= bytes[i] << ((3 - i) * 8);
        else
            ret |= bytes[i] << (i * 8);
    }

    return ret;
}

void int32_to_bytes(uint32_t val, uint8_t* out, bool bigendian) {
    for(int i = 0; i < 4; i++) {
        if(bigendian)
            out[i] = (val >> ((3 - i) * 8)) & 0xff;
        else
            out[i] = (val >> (i * 8)) & 0xff;
    }
}

uint64_t bytes_to_int16(uint8_t* bytes, bool bigendian) {
    uint16_t ret = 0;
    for(int i = 0; i < 2; i++) {
        if(bigendian)
            ret |= bytes[i] << ((1 - i) * 8);
        else
            ret |= bytes[i] << (i * 8);
    }

    return ret;
}

void int16_to_bytes(uint16_t val, uint8_t* out, bool bigendian) {
    for(int i = 0; i < 2; i++) {
        if(bigendian)
            out[i] = (val >> ((1 - i) * 8)) & 0xff;
        else
            out[i] = (val >> (i * 8)) & 0xff;
    }
}

// handle iffyness with preamble processing sometimes being a bit (literally) off
void alt_address_old(uint8_t* packet, uint8_t* altaddr) {
    uint8_t macmess_hi_b[4];
    uint8_t macmess_lo_b[2];
    uint32_t macmess_hi;
    uint16_t macmess_lo;
    uint8_t preserved;

    // get first 6 bytes into 32-bit and 16-bit variables
    memcpy(macmess_hi_b, packet, 4);
    memcpy(macmess_lo_b, packet + 4, 2);

    macmess_hi = bytes_to_int32(macmess_hi_b, true);

    //preserve least 7 bits from hi that will be shifted down to lo
    preserved = macmess_hi & 0x7f;
    macmess_hi >>= 7;

    macmess_lo = bytes_to_int16(macmess_lo_b, true);
    macmess_lo >>= 7;
    macmess_lo = (preserved << 9) | macmess_lo;
    int32_to_bytes(macmess_hi, macmess_hi_b, true);
    int16_to_bytes(macmess_lo, macmess_lo_b, true);
    memcpy(altaddr, &macmess_hi_b[1], 3);
    memcpy(altaddr + 3, macmess_lo_b, 2);
}

bool validate_address(uint8_t* addr) {
    uint16_t bad[] = {0x5555, 0xAAAA, 0x0000, 0xFFFF};
    uint16_t* addr16 = (uint16_t*) addr;

    for (int i = 0; i < 4; i++) {
        if (addr16[0] == bad[i] || addr16[1] == bad[i]) {
            return false;
        }
    }
    return true;
}

bool nrf24_sniff_address(uint8_t maclen, uint8_t* address) {
    uint8_t packet[32] = {0};
    uint8_t packetsize;
    //char printit[65];
    uint8_t status = 0;
    uint8_t rpd_value;
    status = nrf24_rxpacket(packet, &packetsize, true);
    if(status & RX_DR) {
        nrf24_read_reg(REG_RPD, &rpd_value, 1);
        if(rpd_value & 0x01) {
            if(validate_address(packet)) {
                for(int i = 0; i < maclen; i++)
                    address[i] = packet[maclen - 1 - i];
                return true;
            }
        }
    }
    return false;
}

uint8_t nrf24_find_channel(uint8_t* srcmac, uint8_t* dstmac, nrf24_addr_width maclen, nrf24_data_rate rate, uint8_t min_channel, uint8_t max_channel, bool autoinit) {
    uint8_t ping_packet[] = {0x0f, 0x0f, 0x0f, 0x0f};
    uint8_t ch;
  
    find_channel_config.data_rate = rate;
    find_channel_config.rx_addr[0] = srcmac;
    find_channel_config.tx_addr = dstmac;
    find_channel_config.mac_len = maclen;
    find_channel_config.channel = min_channel;

    nrf24_configure(&find_channel_config);

    for(ch = min_channel; ch <= max_channel; ch++) {
        nrf24_write_reg(REG_RF_CH, ch);
        if(nrf24_txpacket(ping_packet, FIND_CHANNEL_PAYLOAD_SIZE, find_channel_config.tx_no_ack)) {
            if(autoinit) {
                FURI_LOG_D("nrf24", "initialisation radio pour le canal %d", ch);
                find_channel_config.channel = ch;
                nrf24_configure(&find_channel_config);
            }
            return ch;
        }
    }

    return max_channel + 1; // Échec
}
