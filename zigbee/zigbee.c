
#include "zigbee.h"

// Array to store discovered devices
ZigBeeDevice discovered_devices[32]; // Up to 32 devices
uint8_t device_count = 0;

// Function to initialize the nRF24 module for ZigBee scanning
void zigbee_scan_init()
{
    nrf24_init();

    nrf24_set_rate(250000);

    FURI_LOG_I(LOG_TAG, "ZigBee Network Scanner Initialized.\n");

    FURI_LOG_I(LOG_TAG, "nfr24 rate : %ld\n", nrf24_get_rate());

}

// Function to scan a single ZigBee channel
bool zigbee_scan_channel(uint8_t channel)
{
    uint8_t packet[32];
    uint8_t packet_size = 0;
    
    // Set the nRF24 to the specified channel
    //rf24_set_chan((channel - 10) * 5);

    nrf24_configure(250000, NULL, NULL, 2, (channel - 10) * 5, false, true);

    // Set nRF24 to RX mode (promiscuous mode)
    nrf24_set_rx_mode();

    for(int8_t i = 0; i < 10; i++) {
        // Wait for a packet (simplified, may need actual timeout handling)
        nrf24_rxpacket(packet, &packet_size, 0); // Listen for incoming packets

        if(packet_size == 0) {
            FURI_LOG_I(LOG_TAG, "No packet received on channel %d\n", nrf24_get_chan());
        }
        else
        {
            // Process the received packet (assuming it's a valid ZigBee packet)
            // For simplicity, we'll assume the MAC address is the first 8 bytes of the packet
            FURI_LOG_I(
                LOG_TAG, "Packet received on channel %d, size: %d bytes\n", channel, packet_size);

            // Extract the MAC address and store the device info
            ZigBeeDevice device;
            device.channel = channel;
            device.rssi =
                0; // Placeholder for RSSI, actual implementation depends on hardware support
            memcpy(device.mac_address, packet, ZIGBEE_MAC_SIZE);

            // Store discovered device
            if(device_count < 32) {
                discovered_devices[device_count++] = device;
                FURI_LOG_I(
                    LOG_TAG,
                    "Device with MAC %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X discovered on channel %d\n",
                    device.mac_address[0],
                    device.mac_address[1],
                    device.mac_address[2],
                    device.mac_address[3],
                    device.mac_address[4],
                    device.mac_address[5],
                    device.mac_address[6],
                    device.mac_address[7],
                    channel);
            }
        }
        furi_delay_ms(1000);
    }
    return true;
}

// Function to scan all ZigBee channels
void zigbee_scan_network()
{
    FURI_LOG_I(LOG_TAG, "Starting ZigBee network scan...\n");
    for(uint8_t channel = ZIGBEE_CHANNEL_MIN; channel <= ZIGBEE_CHANNEL_MAX; channel++) {
        FURI_LOG_I(LOG_TAG, "Scanning channel %d...\n", channel);
        zigbee_scan_channel(channel);
    }
    FURI_LOG_I(LOG_TAG, "ZigBee network scan completed. %d devices found.\n", device_count);
}

// Function to print all discovered devices
void zigbee_print_discovered_devices()
{
    FURI_LOG_I(LOG_TAG, "Discovered ZigBee Devices:\n");
    for(uint8_t i = 0; i < device_count; i++) {
        FURI_LOG_I(LOG_TAG, "Device %d: MAC %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X, Channel %d\n", i+1,
               discovered_devices[i].mac_address[0], discovered_devices[i].mac_address[1],
               discovered_devices[i].mac_address[2], discovered_devices[i].mac_address[3],
               discovered_devices[i].mac_address[4], discovered_devices[i].mac_address[5],
               discovered_devices[i].mac_address[6], discovered_devices[i].mac_address[7],
               discovered_devices[i].channel);
    }
}
