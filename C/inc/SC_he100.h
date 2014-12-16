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

// Transceiver error codes
#define HE_SUCCESS                  0
#define HE_FAILED_OPEN_PORT         1
#define HE_FAILED_CLOSE_PORT        2
#define HE_NOT_A_TTY                3 
#define HE_INVALID_COMMAND          4
#define HE_NOT_READY                5
#define HE_POWER_OFF                6
#define HE_FAILED_TTY_CONFIG        7
#define HE_FAILED_SET_BAUD          8
#define HE_FAILED_FLUSH             9
#define HE_FAILED_CHECKSUM          10
#define HE_FAILED_NACK              11
#define HE_INVALID_BYTE_SEQUENCE    12
#define HE_EMPTY_RESPONSE           13
#define HE_INVALID_POWER_AMP_LEVEL  14
#define HE_INVALID_IF_BAUD_RATE     15
#define HE_INVALID_RF_BAUD_RATE     16
#define HE_INVALID_RX_MOD           17
#define HE_INVALID_TX_MOD           18
#define HE_INVALID_RX_FREQ          19
#define HE_INVALID_TX_FREQ          20
#define HE_INVALID_CALLSIGN         21
#define HE_INVALID_TX_PREAM         22
#define HE_INVALID_TX_POSTAM        23
#define HE_INVALID_RX_PREAM         24
#define HE_INVALID_RX_POSTAM        25
#define HE_INVALID_CRC              26
#define HE_INVALID_DIO_PIN13        27
#define HE_INVALID_RXTX_TEST        28
#define HE_INVALID_EXT              29
#define HE_INVALID_LED              30
#define HE_INVALID_CONFIG           31
#define HE_FAILED_GET_CONFIG        32
#define HE_FAILED_READ              33

extern const char *HE_STATUS[34];
extern const char *CMD_CODE_LIST[32];
extern const char *if_baudrate[6];
extern const char *rf_baudrate[5];

// TTY settings
// baudrate settings are defined in <asm/termbits.h> from <termios.h>
#define BAUDRATE B9600
#define TTYDEVICE "/dev/ttyS2"
#define PARITYBIT ~PARENB // no parity bit
#define BYTESIZE CS8 // 8 data bits
#define STOPBITS ~CSTOPB // 1 stop bit
#define HWFLWCTL ~CRTSCTS // disable hardware flow control
// HELIUM VALUES
#define NOPAY_COMMAND_LENGTH    8
#define WRAPPER_LENGTH          10
#define MIN_POWER_LEVEL         0
#define MAX_POWER_LEVEL         255
#define MAX_TESTED_FRAME        190
#define MAX_FRAME_LENGTH        255
#define HE_ACK                  0x0a
#define HE_NOACK                0xff
// HELIUM HEADER FRAME BYTES
#define HE_SYNC_BYTE_1             0
#define HE_SYNC_BYTE_2             1
#define HE_TX_RX_BYTE              2
#define HE_CMD_BYTE                3
#define HE_LENGTH_BYTE_0           4 // will never be filled, except by ACK/NOACK
#define HE_LENGTH_BYTE             5 
#define HE_HEADER_CHECKSUM_BYTE_1  6
#define HE_HEADER_CHECKSUM_BYTE_2  7
#define HE_FIRST_PAYLOAD_BYTE      8
// Sync and command byte values
#define SYNC1       0x48
#define SYNC2           0x65
#define CMD_TRANSMIT        0x10
#define CMD_RECEIVE         0x20
#define CMD_TELEMETRY_DUMP  0x30 // pending??
#define CMD_PING_RETURN     0x31
#define CMD_CODE_UPLOAD     0x33 // pending??
#define CMD_TOGGLE_PIN      0x34
#define CMD_NOOP                0x01 // noop command, increments command processing counter
#define CMD_RESET               0x02 // reset radio processors and systems
#define CMD_TRANSMIT_DATA       0x03 // send N number of bytes
#define CMD_RECEIVE_DATA        0x04 // receive N number of bytes
#define CMD_GET_CONFIG          0x05 // 20 05 prepends actual config data
#define CMD_SET_CONFIG          0x06 // followed by config bytes
#define CMD_TELEMETRY           0x07 // query a telemetry frame
#define CMD_WRITE_FLASH         0x08 // write flash 16 byte MDF
#define CMD_RF_CONFIGURE        0x09 // Low Level RF Configuration
#define CMD_BEACON_DATA         0x10 // Set Beacon Message
#define CMD_BEACON_CONFIG       0x11 // Set Beacon configuration
#define CMD_READ_FIRMWARE_V     0x12 // read radio firmware revision
// EX {48 65 20 12 REV } float 4 byte revision number
#define CMD_DIO_KEY_WRITE       0x13
#define CMD_FIRMWARE_UPDATE     0x14
#define CMD_FIRMWARE_PACKET     0x15
#define CMD_FAST_SET_PA         0x20
#define CFG_OFF_LOGIC LOW   0x00

// Config options
#define CFG_FRAME_LENGTH    44
#define CFG_PAYLOAD_LENGTH  34
#define CFG_FLASH_LENGTH    16
#define CFG_HUMAN_LENGTH    1500
// Interface BAUD RATE config
#define CFG_IF_BAUD_BYTE    0 // 1st byte
#define CFG_DEF_IF_BAUD     0
#define CFG_IF_BAUD_9600    0
#define CFG_IF_BAUD_19200   1
#define CFG_IF_BAUD_38400   2
#define CFG_IF_BAUD_76800   3
#define CFG_IF_BAUD_115200  4
#define MAX_IF_BAUD_RATE    4
#define MIN_IF_BAUD_RATE    0
// PA config
#define CFG_PA_BYTE         1 // 2nd byte 
#define MAX_PA_LEVEL        255
#define MIN_PA_LEVEL        0
// RF BAUD rate config
#define CFG_RF_RX_BAUD_BYTE 2 // 3rd byte 
#define CFG_RF_TX_BAUD_BYTE 3 // 4th byte 
#define CFG_DEF_RF_BAUDRATE 1
#define CFG_RF_BAUD_1200    0
#define CFG_RF_BAUD_9600    1
#define CFG_RF_BAUD_19200   2
#define CFG_RF_BAUD_38400   3
#define MAX_RF_BAUD_RATE    3
#define MIN_RF_BAUD_RATE    0
// MODULATION config
#define CFG_RX_MOD_BYTE     4 // 5th byte
#define CFG_TX_MOD_BYTE     5 // 6th byte
#define CFG_RX_MOD_DEFAULT  0x00 // GFSK
#define CFG_TX_MOD_DEFAULT  0x00 // GFSK
// RX TX FREQ config
#define CFG_RX_FREQ_BYTE1   6 // 7th byte
#define CFG_RX_FREQ_BYTE2   7 // 8th byte
#define CFG_RX_FREQ_BYTE3   8 // 8th byte
#define CFG_RX_FREQ_BYTE4   9 // 8th byte
#define CFG_RX_FREQ_DEFAULT 144200L
#define CFG_TX_FREQ_BYTE1   10 // 11th byte
#define CFG_TX_FREQ_BYTE2   11 // 12th byte
#define CFG_TX_FREQ_BYTE3   12 // 12th byte
#define CFG_TX_FREQ_BYTE4   13 // 12th byte
#define CFG_TX_FREQ_DEFAULT 431000L
#define MAX_UPPER_FREQ      450000L
#define MIN_UPPER_FREQ      400000L
#define MAX_LOWER_FREQ      150000L
#define MIN_LOWER_FREQ      120000L
// CALLSIGN config
#define CFG_SRC_CALL_BYTE   14 // 15th byte  
#define CFG_DST_CALL_BYTE   20 // 21st byte
#define CFG_SRC_CALL_DEF    "VA3ORB"
#define CFG_DST_CALL_DEF    "VE2CUA"
#define CFG_CALLSIGN_LEN    6

// PREAMBLE/POSTAMBLE config
#define CFG_TX_PREAM_BYTE   26 // 27th byte
#define CFG_TX_PREAM_DEF    0
#define CFG_TX_PREAM_MIN    0
#define CFG_TX_PREAM_MAX    10
#define CFG_TX_POSTAM_BYTE  28 // 29th byte
#define CFG_TX_POSTAM_DEF   0
#define CFG_TX_POSTAM_MIN   0
#define CFG_TX_POSTAM_MAX   10
#define CFG_RX_PREAM_BYTE   26 // 27th byte
#define CFG_RX_PREAM_DEF    0
#define CFG_RX_PREAM_MIN    0
#define CFG_RX_PREAM_MAX    10
#define CFG_RX_POSTAM_BYTE   28 // 29th byte
#define CFG_RX_POSTAM_DEF    0
#define CFG_RX_POSTAM_MIN    0
#define CFG_RX_POSTAM_MAX    10
// TODO these are 16-bit variables, fix them

#define CFG_FUNCTION_CONFIG_BYTE    30
#define CFG_FUNCTION_CONFIG_LENGTH  2 // bytes

#define CFG_FUNCTION_CONFIG2_BYTE   32
#define CFG_FUNCTION_CONFIG2_LENGTH 1 // byte

// EXT Functions config
#define CFG_EXT_BYTE          33 // 34th byte
#define CFG_EXT_DEF           0x00  
#define CFG_EXT_OFF           0x00
#define CFG_EXT_PING_ON       0x10
#define CFG_EXT_CODEUPLOAD_ON 0x20
#define CFG_EXT_RESET_ON      0x40

// DEPRECATED
// RX CRC config
#define CFG_RX_CRC_BYTE     30 // 31st byte
#define CFG_RX_CRC_ON       0x43
#define CFG_RX_CRC_OFF      0x03
// RX CRC config// DIO - Pin 13 config
#define CFG_DIO_PIN13_BYTE      30 // 31st byte
#define CFG_DIO_PIN13_OFF       0x43
#define CFG_DIO_PIN13_TXRXS     0x47 
#define CFG_DIO_PIN13_2p5HZ     0x4b
#define CFG_DIO_PIN13_RXTOG     0x4f
// LED config
#define CFG_LED_BYTE 30  // 38th byte in byte array
#define CFG_LED_PS  0x41 // 2.5 second pulse
#define CFG_LED_TX  0x42 // flash on transmit
#define CFG_LED_RX  0x43 // flash on receive
// TX Test CW config
#define CFG_RXTX_TEST_CW_BYTE 32 // 33rd byte
#define CFG_RXTX_TEST_CW_DEF  0x00 
#define CFG_RXTX_TEST_CW_OFF  0x00 
#define CFG_TX_TEST_CW_ON     0x02 
#define CFG_RX_TEST_CW_ON     0x04
// END DEPRECATED

/**
 *  function_config bit values
 **/
struct function_config {
    unsigned led:2;
    unsigned pin13:2;           
    unsigned pin14:2;           // pin 14
    unsigned crc_tx:1;           // enable TX CRC
    unsigned crc_rx:1;           // enable RX CRC
    unsigned telemetry_dump_status:1;     // enable telemetry dump
    unsigned telemetry_rate:2;     // logging rate 0 1/10 Hz, 1 1 Hz, 2 2 Hz, 3 4 Hz
    unsigned telemetry_status:1;     // enable telemetry logging 
    unsigned beacon_radio_reset_status:1;        // enable radio reset
    unsigned beacon_code_upload_status:1;        // enable code upload
    unsigned beacon_oa_cmd_status:1;        // enable OA Commands
    unsigned beacon_0:1;
};

/**
 * function_config2 bit values
 **/
struct function_config2 {
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
  uint32_t 	                rx_freq; // Channel Rx Frequency default 144200
  uint32_t 	                tx_freq; // Channel Tx Frequency default 431000
  unsigned char	            source_callsign[7]; // VA3ORB, default NOCALL
  unsigned char             destination_callsign[7]; // VE2CUA, default CQ
  uint16_t                  tx_preamble; // AX25 Mode Tx Preamble byte length (0x00 = 20 flags)
  uint16_t                  tx_postamble; // AX25 Mode Tx Postamble byte length (0x00 = 20 flags)
  struct function_config    function_config; 
  struct function_config2   function_config2;
  uint8_t                   ext_conf_setting;
};

extern const char * CFG_FC_LED[4];
extern const char * CFG_FC_PIN13[4];
extern const char * CFG_FC_PIN14[4];
extern const char * CFG_FC_RX_CRC[2];
extern const char * CFG_FC_TX_CRC[2];
extern const char * CFG_FC_TELEMETRY[2];
extern const char * CFG_FC_TELEMETRY_RATE[4];
extern const char * CFG_FC_TELEMETRY_DUMP[2];
extern const char * CFG_FC_BEACON_OA_COMMANDS[2];
extern const char * CFG_FC_BEACON_CODE_UPLOAD[2];
extern const char * CFG_FC_BEACON_RESET[2];

// function_config
#define CFG_FC_LED_OFFLOGICLOW              0 //0b00
#define CFG_FC_LED_PULSE                    1 //0b01
#define CFG_FC_LED_TXTOG                    2 //0b10
#define CFG_FC_LED_RXTOG                    3 //0b11
#define CFG_FC_LED_DEFAULT                  CFG_FC_LED_OFFLOGICLOW

#define CFG_FC_PIN13_OFFLOGICLOW            0 //0b00
#define CFG_FC_PIN13_TXRXSWITCH             1 //0b01 // approx 0.35 seconds high, depends on pre/postamble
#define CFG_FC_PIN13_2P5HZWDT               2 //0b10
#define CFG_FC_PIN13_RXPACKETTOG            3 //0b11
#define CFG_FC_PIN13_DEFAULT                CFG_FC_PIN_13_OFFLOGICLOW

#define CFG_FC_PIN14_OFFLOGICLOW            0 //0b00
#define CFG_FC_PIN14_DIOOVERAIR_ON          1 //0b01
#define CFG_FC_PIN14_DIOOVERAIR_A           2 //0b10 // latching high
#define CFG_FC_PIN14_DIOOVERAIR_B           3 //0b11 // toggle, 72 ms high
#define CFG_FC_PIN14_DEFAULT                CFG_FC_PIN14_OFFLOGICLOW

#define CFG_FC_RX_CRC_OFF                   0 //0b00
#define CFG_FC_RX_CRC_ON                    1 //0b01
#define CFG_FC_RX_CRC_DEFAULT               CFG_FC_RX_CRC_OFF

#define CFG_FC_TX_CRC_OFF                   0 //0b00
#define CFG_FC_TX_CRC_ON                    1 //0b01
#define CFG_FC_TX_CRC_DEFAULT               CFG_FC_TX_CRC_OFF

#define CFG_FC_TELEMETRY_OFF                0 //0b00
#define CFG_FC_TELEMETRY_ON                 1 //0b01
#define CFG_FC_TELEMETRY_DEFAULT            CFG_FC_TELEMETRY_OFF

#define CFG_FC_TELEMETRY_RATE_P10HZ         0 //0b00
#define CFG_FC_TELEMETRY_RATE_1HZ           1 //0b01
#define CFG_FC_TELEMETRY_RATE_2HZ           2 //0b10
#define CFG_FC_TELEMETRY_RATE_3HZ           3 //0b11
#define CFG_FC_TELEMETRY_RATE_DEFAULT       CFG_FC_TELEMETRY_RATE_P10HZ

#define CFG_FC_TELEMETRY_DUMP_OFF           0 //0b00
#define CFG_FC_TELEMETRY_DUMP_ON            1 //0b01
#define CFG_FC_TELEMETRY_DUMP_DEFAULT       CFG_FC_TELEMETRY_DUMP_OFF

#define CFG_FC_BEACON_OA_COMMANDS_OFF       0 //0b00
#define CFG_FC_BEACON_OA_COMMANDS_ON        1 //0b01
#define CFG_FC_BEACON_OA_COMMANDS_DEFAULT   CFG_FC_BEACON_OA_COMMANDS_OFF

#define CFG_FC_BEACON_CODE_UPLOAD_OFF       0 //0b00
#define CFG_FC_BEACON_CODE_UPLOAD_ON        1 //0b01
#define CFG_FC_BEACON_CODE_UPLOAD_DEFAULT   CFG_FC_BEACON_CODE_UPLOAD_OFF

#define CFG_FC_BEACON_CODE_RESET_OFF         0 //0b00
#define CFG_FC_BEACON_CODE_RESET_ON        1 //0b01
#define CFG_FC_BEACON_CODE_RESET_DEFAULT    CFG_FC_BEACON_CODE_RESET_OFF

// function_config2
#define CFG_FC_RAFC_OFF                     0 //0b00
#define CFG_FC_RAFC_ON                      1 //0b01
#define CFG_FC_RAFC_DEFAULT                 CFG_FC_RAFC_ON

#define CFG_FC_RXCW_OFF                     0 //0b00
#define CFG_FC_RXCW_ON                      1 //0b01
#define CFG_FC_RXCW_DEFAULT                 CFG_FC_RXCW_ON

#define CFG_FC_TXCW_OFF                     0 //0b00
#define CFG_FC_TXCW_ON                      1 //0b01
#define CFG_FC_TXCW_DEFAULT                 CFG_FC_TXCW_ON

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

/**
 * Function to configure serial interface
 * @param fdin - the file descriptor representing the serial device
 * @return int - exit status 
 * REF: http://man7.org/linux/man-pages/man3/termios.3.html
 * REF: http://www.unixguide.net/unix/programming/3.6.2.shtml
 */
int HE100_configureInterface (int);

/* Function to open HE100 device on configured seial port address */
int HE100_openPort (void);

/* Function to close serial device connection at given file descriptor */
int HE100_closePort (int);

/* Function to write a char array to a serial device at given file descriptor */
int HE100_write (int fdin, unsigned char *bytes, size_t size);

/**
 * struct to hold values of fletcher checksum
 */
//typedef struct HE100_checksum {} HE100_checksum;

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

/** Provide signal handling for HE100_read **/
//volatile sig_atomic_t stop;
//void inthand (int signum) { stop = 1; }

/**
 * Function to read bytes in single-file from the serial device and
 * append them to and return a response array
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

//int HE100_validateConfig (struct he100_settings he100_new_settings, unsigned char * set_config_payload );
int HE100_prepareConfig (unsigned char * prepared_bytes, struct he100_settings settings);
int HE100_validateConfig (struct he100_settings he100_new_settings);

/* Function to configure the Helium board based on altered input struct he100_settings */
/* validation will occur here, and if valid values have passed constraints, apply the settings */
int HE100_setConfig (int fdin, struct he100_settings he100_new_settings);

void HE100_printSettings(FILE* fdout, struct he100_settings settings);

#endif
