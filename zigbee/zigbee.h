#ifndef ZIGBEE_H
#define ZIGBEE_H

#include <stdint.h>
#include <stdbool.h>
#include "libnrf24/nrf24.h" // Includes the nRF24 functions you provided earlier
#include "nrf24tool.h"


#define ZIGBEE_CHANNEL_MIN 11 // ZigBee channels start at 11
#define ZIGBEE_CHANNEL_MAX 26 // ZigBee channels end at 26

#define ZIGBEE_MAC_SIZE 8 // ZigBee devices use 8-byte MAC addresses

// Structure to hold discovered device information
typedef struct
{
    uint8_t mac_address[ZIGBEE_MAC_SIZE];
    uint8_t channel;
    int8_t rssi; // Received signal strength indicator
} ZigBeeDevice;

// Function to initialize the nRF24 module for ZigBee scanning
void zigbee_scan_init();

// Function to scan a single ZigBee channel
bool zigbee_scan_channel(uint8_t channel);

// Function to scan all ZigBee channels
void zigbee_scan_network();

// Function to print all discovered devices
void zigbee_print_discovered_devices();

#endif