#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_spi.h>

#define R_REGISTER         0x00
#define W_REGISTER         0x20
#define REGISTER_MASK      0x1F
#define ACTIVATE           0x50
#define R_RX_PL_WID        0x60
#define R_RX_PAYLOAD       0x61
#define W_TX_PAYLOAD       0xA0
#define W_TX_PAYLOAD_NOACK 0xB0
#define W_ACK_PAYLOAD      0xA8
#define FLUSH_TX           0xE1
#define FLUSH_RX           0xE2
#define REUSE_TX_PL        0xE3
#define RF24_NOP           0xFF

#define REG_CONFIG      0x00
#define REG_EN_AA       0x01
#define REG_EN_RXADDR   0x02
#define REG_SETUP_AW    0x03
#define REG_SETUP_RETR  0x04
#define REG_DYNPD       0x1C
#define REG_FEATURE     0x1D
#define REG_RF_SETUP    0x06
#define REG_STATUS      0x07
#define REG_RX_ADDR_P0  0x0A
#define REG_RX_ADDR_P1  0x0B
#define REG_RX_ADDR_P2  0x0C
#define REG_RX_ADDR_P3  0x0D
#define REG_RX_ADDR_P4  0x0E
#define REG_RX_ADDR_P5  0x0F
#define REG_RF_CH       0x05
#define REG_TX_ADDR     0x10
#define REG_FIFO_STATUS 0x17

#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define RX_DR    0x40
#define TX_DS    0x20
#define MAX_RT   0x10

#define MAX_CHANNEL      125
#define MAX_PIPE         5
#define MIN_PAYLOAD_SIZE 1
#define MAX_PAYLOAD_SIZE 32
#define MAX_CRC_LENGHT   2
#define MAX_MAC_SIZE     5
#define MIN_ARD_SIZE     250 // Auto Retransmit Delay in µs
#define MAX_ARD_SIZE     4000 // Auto Retransmit Delay in µs
#define MAX_ARC_SIZE     15 //Auto Retransmit Count

#define nrf24_TIMEOUT 500
#define nrf24_CE_PIN  &gpio_ext_pb2
#define nrf24_HANDLE  furi_hal_spi_bus_handle_external

typedef enum {
    DATA_RATE_1MBPS = 0, // 1 Mbps (default)
    DATA_RATE_2MBPS, // 2 Mbps
    DATA_RATE_250KBPS // 250 kbps
} nrf24_data_rate;

typedef enum {
    TX_POWER_M18DBM = 0, // -18 dBm (lowest power)
    TX_POWER_M12DBM, // -12 dBm
    TX_POWER_M6DBM, // -6 dBm
    TX_POWER_0DBM // 0 dBm (highest power)
} nrf24_tx_power;

typedef enum {
    MODE_IDLE = 0, // Power Down Mode (low power consumption)
    MODE_STANDBY, // Standby Mode (ready to switch to TX or RX)
    MODE_RX, // Receive Mode (listening for incoming data)
    MODE_TX // Transmit Mode (sending data)
} nrf24_mode;

typedef enum {
    ADDR_WIDTH_2_BYTES = 0,
    ADDR_WIDTH_3_BYTES,
    ADDR_WIDTH_4_BYTES,
    ADDR_WIDTH_5_BYTES
} nrf24_addr_width;

typedef struct NRF24L01_Config {
    uint8_t channel; // RF channel (0 - 125)
    nrf24_data_rate data_rate; // Transmission speed: 0 -> 1 Mbps, 1 -> 2 Mbps, 2 -> 250 kbps
    nrf24_tx_power tx_power; // Transmission power: 0-> -18 dBm, 1 -> -12 dBm, 2-> -6 dBm, 3-> 0 dBm
    uint8_t crc_length; // CRC length: 1 or 2 bytes
    nrf24_addr_width mac_len; // Address width: 3, 4 or 5 bytes
    uint8_t arc; // Auto retransmit count (0-15)
    uint16_t ard; // Retransmit delay (250 µs to 4000 µs)
    bool auto_ack; // Auto ACK aka "Enhanced ShockBurst"
    bool dynamic_payload; // Dynamic payload (require auto_ack)
    bool ack_payload; // Return ACK + payload (require dynamic payload)
    bool tx_no_ack; // Send "NO_ACK" flag with transmission
    bool enable_crc; // CRC enabled (forced enable if auto_ack enabled)

    uint8_t* tx_addr; // Transmission address
    uint8_t* rx_addr; // Reception address (pipe 0) (equal to tx_adress if auto_ack enabled)

    uint8_t payload_size; // Fixed payload size (1 to 32 bytes) (unset if dynamic_payload is enabled)
} NRF24L01_Config;

/* Low level API */

/** Write device register
 *
 * @param      reg     - register
 * @param      data    - data to write
 *
 * @return     device status
 */
uint8_t nrf24_write_reg(uint8_t reg, uint8_t data);

/** Write buffer to device register
 *
 * @param      reg     - register
 * @param      data    - data to write
 * @param      size    - size of data to write
 *
 * @return     device status
 */
uint8_t nrf24_write_buf_reg(uint8_t reg, uint8_t* data, uint8_t size);

/** Read device register
 *
 * @param      reg     - register
 * @param[out] data    - pointer to data
 *
 * @return     device status
 */
uint8_t nrf24_read_reg(uint8_t reg, uint8_t* data, uint8_t size);

/** Set the operational mode
 * 
 * MODE_IDLE = 0,  Power Down Mode (low power consumption)
 * MODE_STANDBY,   Standby Mode (ready to switch to TX or RX)
 * MODE_RX,        Receive Mode (listening for incoming data)
 * MODE_TX         ransmit Mode (sending data)
 * @return     device status
 */

uint8_t nrf24_set_mode(nrf24_mode mode);

/*=============================================================================================================*/

/* High level API */

/** Must call this before using any other nrf24 API
 * 
 */
void nrf24_init();

/** Must call this when we end using nrf24 device
 * 
 */
void nrf24_deinit();

/** Send flush rx command
 *
 *
 * @return     device status
 */
uint8_t nrf24_flush_rx();

/** Send flush tx command
 *
 *
 * @return     device status
 */
uint8_t nrf24_flush_tx();

/** Gets the RX packet length in data pipe 0
 * 
 *             pipe - pipe index (0..5)
 * @return     packet length in data pipe 0
 */
uint8_t nrf24_get_packetlen(uint8_t pipe);

/** Sets the RX packet length in data pipe 0
 * 
 * @param      len - length to set
 * 
 * @return     device status
 */
uint8_t nrf24_set_packetlen(uint8_t len);

/** Gets configured length of MAC address
 *
 * 
 * @return     MAC address length
 */
nrf24_addr_width nrf24_get_maclen();

/** Sets configured length of MAC address
 *
 * @param      maclen - length to set MAC address to, must be greater than 1 and less than 6
 * 
 * @return     MAC address length
 */
uint8_t nrf24_set_maclen(nrf24_addr_width maclen);

/** Gets the current status flags from the STATUS register
 * 
 * 
 * @return     status flags
 */
uint8_t nrf24_status();

/** Gets the current transfer rate
 * 
 * 
 * @return     transfer rate in bps
 */
nrf24_data_rate nrf24_get_rate();

/** Sets the transfer rate
 *
 * @param      rate - the transfer rate in bps
 * 
 * @return     device status
 */
uint8_t nrf24_set_rate(nrf24_data_rate rate);

/** Gets the current channel
 * In nrf24, the channel number is multiplied times 1MHz and added to 2400MHz to get the frequency
 * 
 * 
 * @return     channel
 */
uint8_t nrf24_get_chan();

/** Sets the channel
 *
 * @param      frequency - the frequency in hertz
 * 
 * @return     device status
 */
uint8_t nrf24_set_chan(uint8_t chan);

/** Gets the source mac address
 *
 * @param[out] mac - the source mac address
 * 
 * @return     device status
 */
uint8_t nrf24_get_rx_mac(uint8_t* mac);

/** Sets the source mac address
 *
 * @param      mac - the mac address to set
 * @param      size - the size of the mac address (2 to 5)
 * 
 * @return     device status
 */
uint8_t nrf24_set_rx_mac(uint8_t* mac, uint8_t size);

/** Gets the dest mac address
 *
 * @param[out] mac - the source mac address
 * 
 * @return     device status
 */
uint8_t nrf24_get_tx_mac(uint8_t* mac);

/** Sets the dest mac address
 *
 * @param      mac - the mac address to set
 * @param      size - the size of the mac address (2 to 5)
 * 
 * @return     device status
 */
uint8_t nrf24_set_tx_mac(uint8_t* mac, uint8_t size);

/** Reads RX packet
 *
 * @param[out] packet - the packet contents
 * @param[out] ret_packetsize - size of the received packet
 * @param      packet_size: >1 - size, 1 - packet length is determined by RX_PW_P0 register, 0 - it is determined by dynamic payload length command
 * 
 * @return     device status
 */
uint8_t nrf24_rxpacket(uint8_t* packet, uint8_t* ret_packetsize, uint8_t packet_size);

/** Sends TX packet
 *
 * @param      packet - the packet contents
 * @param      size - packet size
 * @param      ack - boolean to determine whether an ACK is required for the packet or not
 * 
 * @return     device status
 */
uint8_t nrf24_txpacket(uint8_t* payload, uint8_t size, bool ack);

/** Configure the radio
 * This is not comprehensive, but covers a lot of the common configuration options that may be changed 
 */
void nrf24_configure(NRF24L01_Config* config);

/** Configures the radio for "promiscuous mode" and primes it for rx
 * This is not an actual mode of the nrf24, but this function exploits a few bugs in the chip that allows it to act as if it were.
 * See http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html for details.
 * @param      channel - channel to tune to
 * @param      rate - transfer rate in Mbps (1 or 2) 
 */
void nrf24_init_promisc_mode(uint8_t channel, uint8_t rate);

/** Listens for a packet and returns first possible address sniffed
 * Call this only after calling nrf24_init_promisc_mode
 * @param      maclen - length of target mac address
 * @param[out] addresses - sniffed address
 * 
 * @return     success
 */
bool nrf24_sniff_address(uint8_t maclen, uint8_t* address);

/** Sends ping packet on each channel for designated tx mac looking for ack
 * 
 * @param      srcmac - source address
 * @param      dstmac - destination address
 * @param      maclen - length of address
 * @param      rate - transfer rate in Mbps (1 or 2) 
 * @param      min_channel - channel to start with
 * @param      max_channel - channel to end at
 * @param      autoinit - if true, automatically configure radio for this channel
 * 
 * @return     channel that the address is listening on, if this value is above the max_channel param, it failed
 */
uint8_t nrf24_find_channel(
    uint8_t* srcmac,
    uint8_t* dstmac,
    uint8_t maclen,
    uint8_t rate,
    uint8_t min_channel,
    uint8_t max_channel,
    bool autoinit);

/** Converts 64 bit value into uint8_t array
 * @param      val  - 64-bit integer
 * @param[out] out - bytes out
 * @param      bigendian - if true, convert as big endian, otherwise little endian
 */
void int64_to_bytes(uint64_t val, uint8_t* out, bool bigendian);

/** Converts 32 bit value into uint8_t array
 * @param      val  - 32-bit integer
 * @param[out] out - bytes out
 * @param      bigendian - if true, convert as big endian, otherwise little endian
 */
void int32_to_bytes(uint32_t val, uint8_t* out, bool bigendian);

/** Converts uint8_t array into 32 bit value
 * @param      bytes  - uint8_t array
 * @param      bigendian - if true, convert as big endian, otherwise little endian
 * 
 * @return     32-bit value
 */
uint32_t bytes_to_int32(uint8_t* bytes, bool bigendian);
