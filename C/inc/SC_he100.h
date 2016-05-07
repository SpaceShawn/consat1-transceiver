#ifndef SC_HE100_H_
#define SC_HE100_H_

/*
 * =====================================================================================
 *
 *       Filename:  SC_he100.h
 *
 *    Description:  Header file for he100 library
 *
 *        Version:  1.0
 *        Created:  13-11-09 01:14:59 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  SHAWN BULGER (),
 *   Organization:
 *
 * =====================================================================================
 */

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <HE100_constants.h>    // contains all constants specific to hardware bytes
#include <SC_serial.h>          // contains serial access functionality

#define BE 1
#define LE 0

// function_config bit values - 16-bit
struct function_config { 
    unsigned led:2;
    unsigned pin13:2;           
    unsigned pin14:2;
    unsigned crc_tx:1;
    unsigned crc_rx:1; 
    unsigned telemetry_dump_status:1; // enable telemetry dump
    unsigned telemetry_rate:2; // logging rate 0 1/10 Hz, 1 1 Hz, 2 2 Hz, 3 4 Hz
    unsigned telemetry_status:1;  // enable telemetry logging 
    unsigned beacon_radio_reset_status:1; // enable radio reset
    unsigned beacon_code_upload_status:1; // enable code upload
    unsigned beacon_oa_cmd_status:1; // enable OA Commands
    unsigned beacon_0:1;
};

// function_config2 bit values - 16-bit
struct function_config2 {
    unsigned t0:4;
    unsigned t4:4;
    unsigned t8:4;
    unsigned tbd:1;
    unsigned txcw:1;
    unsigned rxcw:1;
    unsigned rafc:1;
};

struct he100_settings {
    uint8_t                   interface_baud_rate; // Radio Interface Baud Rate (9600=0x00)
    uint8_t                   tx_power_amp_level; // Tx Power Amp Level (min=0x00, max=0xFF)
    uint8_t                   rx_rf_baud_rate; // Radio RX RF Baud Rate (9600=0x00)
    uint8_t                   tx_rf_baud_rate; // Radio TX RF Baud Rate (9600=0x00)
    uint8_t                   rx_modulation; // (0x00 = GFSK)
    uint8_t                   tx_modulation; // (0x00 = GFSK)
    uint32_t 	              rx_freq; // Channel Rx Frequency default 144200
    uint32_t 	              tx_freq; // Channel Tx Frequency default 431000
    unsigned char	          source_callsign[7]; // VA3ORB, default NOCALL
    unsigned char             destination_callsign[7]; // VE2CUA, default CQ
    uint16_t                  tx_preamble; // AX25 Mode Tx Preamble byte length (0x00 = 20 flags)
    uint16_t                  tx_postamble; // AX25 Mode Tx Postamble byte length (0x00 = 20 flags)
    uint16_t                  rx_preamble; // AX25 Mode Rx Preamble byte length (0x00 = 20 flags)
    uint16_t                  rx_postamble; // AX25 Mode Rx Postamble byte length (0x00 = 20 flags)
    struct function_config    function_config; 
    struct function_config2   function_config2;
};

char endian(void);

/**
 * Telemetry structure
 */
typedef struct telem_type {
    uint16_t op_counter;
    uint16_t msp430_temp;
    uint8_t time_count[3];
    uint8_t rssi;
    uint32_t bytes_received;
    uint32_t bytes_transmitted;
} TELEMETRY_STRUCTURE_type;

/**
 *   The Low Level RF Structure
 */
typedef struct {
    uint8_t front_end_level; //0 to 63 Value
    uint8_t tx_power_amp_level; //0 to 255 value, non-linear
    uint32_t tx_frequency_offset; //Up to 20 kHz
    uint32_t rx_frequency_offset; //Up to 20 kHz
} RADIO_RF_CONFIGURATION_TYPE;

/* Function to write a char array to a serial device at given file descriptor */
int HE100_write (int fdin, unsigned char *bytes, size_t size);

/** Optimized Fletcher Checksum
 * 16-bit implementation of the Fletcher Checksum
 * returns two 8-bit sums
 * @param data - uint8_t const - data on which to perform checksum
 * @param bytes - size_t - number of bytes to process
 * inspired by http://en.wikipedia.org/wiki/Fletcher%27s_checksum#Optimizations
 */
struct HE100_checksum HE100_fletcher16 (unsigned char *data, size_t bytes);

/**
 * Function to parse a given frame, validate it, and write its payload to pipe
 * @param response - the frame data to be validated
 * @param length - the entire length of the frame in bytes
 */
int HE100_validateFrame (unsigned char *response, size_t length);

/* Function to dump a given array to a given file descriptor */
int HE100_dumpBinary (FILE *fdout, unsigned char *bytes, size_t size);
void HE100_dumpHex (FILE *fdout, unsigned char *bytes, size_t size);

/**
 * The HE100_read function obtains communication payloads from the 
 * serial device and returns an execution status.
 *
 * @param fdin - the file descriptor representing the serial device
 * @param payload - a buffer you pass with 255 bytes of memory in which to place
 *  the response data
 * @return - the length of the payload read
 */
int HE100_read (int fdin, time_t timeout, unsigned char * payload);

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of data stream
 */
int HE100_prepareTransmission(unsigned char *payload, unsigned char *prepared_transmission, size_t length, unsigned char *command);
//unsigned char * HE100_prepareTransmission (unsigned char *payload, size_t length, unsigned char *command);

/* Function to ensure byte-by-byte that we are receiving a HE100 frame */
int HE100_referenceByteSequence(unsigned char *response, int position);

/**
 * Function to decode validated and extracted data from response
 * @param response - the response data to interpret
 * @param length - the length of the data in bytes
 */
int HE100_interpretResponse (unsigned char *response, size_t length);

/**
 * Function to take radio command, payload and payload length and write to radio
 * @param fdin
 * @param payload
 * @param payload_length
 * @param command
 */
int HE100_dispatchTransmission(int fdin, unsigned char *payload, size_t payload_length, unsigned char *command);

/**
 * Function to return NOOP byte sequence
 * no arguments
 */
int HE100_NOOP(int fdin);

/**
 * Function returning byte sequence to set the beacon message
 * unsigned char *beacon_message_payload message to transmit
 */
int HE100_transmitData (int fdin, unsigned char *transmit_data_payload, size_t transmit_data_len);

/**
 * Function returning byte sequence to enable beacon on given interval
 * int beacon_interval interval in seconds
 */
int HE100_setBeaconInterval (int fdin, int beacon_interval);

/**
 * Function returning byte sequence to set the beacon message
 * unsigned char *beacon_message_payload message to transmit
 */
int HE100_setBeaconMessage (int fdin, unsigned char *set_beacon_message_payload, size_t beacon_message_len);

/**
 * Function returning byte sequence to amplify power based
 * on input int power_level
 * int power_level decimal value from 0-255 (0%-100%)
 */
int HE100_fastSetPA (int fdin, int power_level);

/**
 * Function returning byte sequence to soft reset HE100 board and restore flash settings
 * no arguments
 */
int HE100_softReset(int fdin);

/**
 * Function returning byte sequence to return firmware version
 * no arguments
 */
int HE100_readFirmwareRevision(int fdin);

/* Function to return an array of config struct from Helium 100 */
struct he100_settings HE100_collectConfig (unsigned char * buffer);
//int HE100_getConfig (int fdin);
int HE100_getConfig (int fdin, struct he100_settings * settings);

// swap endianness of anything larger than 1 byte in the config array
int HE100_configEndianness (struct he100_settings & settings);

//int HE100_validateConfig (struct he100_settings he100_new_settings, unsigned char * set_config_payload );
int HE100_prepareConfig (unsigned char &prepared_bytes, struct he100_settings settings);
int HE100_validateConfig (struct he100_settings he100_new_settings);

/* Function to configure the Helium board based on altered input struct he100_settings */
/* validation will occur here, and if valid values have passed constraints, apply the settings */
int HE100_setConfig (int fdin, struct he100_settings he100_new_settings);

void HE100_printSettings(FILE* fdout, struct he100_settings settings);

#endif
