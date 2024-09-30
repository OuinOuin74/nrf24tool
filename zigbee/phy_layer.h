#ifndef __PHY_LAYER_H__
#define __PHY_LAYER_H__

#include <stdint.h>

#include "../libnrf24/nrf24.h"

// Define constants for ZigBee PHY
#define ZIGBEE_PREAMBLE_SIZE 4
#define ZIGBEE_SFD 0xA7
#define ZIGBEE_MAX_PAYLOAD_SIZE 127  // ZigBee standard max payload size
#define ZIGBEE_FCS_SIZE 2

// PHY Frame structure for ZigBee
typedef struct {
    uint8_t preamble[ZIGBEE_PREAMBLE_SIZE];  // Preamble (4 bytes of 0x00)
    uint8_t sfd;                             // Start Frame Delimiter (0xA7)
    uint8_t frame_length;                    // Length of the PHY payload (including MAC header + data)
    
    // MAC Header
    uint16_t frame_control;                  // MAC Frame Control field (2 bytes)
    uint8_t sequence_number;                 // Sequence number for the frame
    uint16_t dest_pan_id;                    // Destination PAN ID
    uint16_t dest_address;                   // Destination short address (16 bits)
    uint16_t src_address;                    // Source short address (16 bits)
    
    // Payload (MAC Payload / NWK Payload / APS Payload)
    uint8_t payload[ZIGBEE_MAX_PAYLOAD_SIZE]; // Variable payload (up to 127 bytes)

    // FCS (Frame Check Sequence)
    uint16_t fcs;                            // Frame Check Sequence (CRC-16)

} ZigBeePHYFrame;


// Function to compute Frame Check Sequence (FCS) using CRC-16
uint16_t compute_fcs(uint8_t* data, uint8_t length)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++)
    {
        crc ^= (data[i] << 8);
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;  // CRC-16-CCITT polynomial
            } 
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

#endif // __PHY_LAYER_H__