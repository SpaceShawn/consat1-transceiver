#ifndef HE100_CONSTANTS_H_
#define HE100_CONSTANTS_H_

/*
 * =====================================================================================
 *
 *       Filename:  SC_he100.h
 *
 *    Description:  Contains definitions of constants for return statuses, as well as 
 *                  constants that define parameters for the hardware to operate,
 *                  such as the tested size of payload and command bytes
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
#define CFG_RX_MOD_GFSK     CFG_RX_MOD_DEFAULT
#define CFG_TX_MOD_GFSK     CFG_TX_MOD_DEFAULT
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
#define CFG_FUNCTION_CONFIG2_LENGTH 2 // bytes


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

// function_config
#define CFG_FC_LED_OFFLOGICLOW              0 //0b00
#define CFG_FC_LED_PULSE                    1 //0b01
#define CFG_FC_LED_TXTOG                    2 //0b10
#define CFG_FC_LED_RXTOG                    3 //0b11
#define CFG_FC_LED_DEFAULT                  CFG_FC_LED_OFFLOGICLOW
extern const char * CFG_FC_LED[4];

#define CFG_FC_PIN13_OFFLOGICLOW            0 //0b00
#define CFG_FC_PIN13_TXRXSWITCH             1 //0b01 // approx 0.35 seconds high, depends on pre/postamble
#define CFG_FC_PIN13_2P5HZWDT               2 //0b10
#define CFG_FC_PIN13_RXPACKETTOG            3 //0b11
#define CFG_FC_PIN13_DEFAULT                CFG_FC_PIN_13_OFFLOGICLOW
extern const char * CFG_FC_PIN13[4];

#define CFG_FC_PIN14_OFFLOGICLOW            0 //0b00
#define CFG_FC_PIN14_DIOOVERAIR_ON          1 //0b01
#define CFG_FC_PIN14_DIOOVERAIR_A           2 //0b10 // latching high
#define CFG_FC_PIN14_DIOOVERAIR_B           3 //0b11 // toggle, 72 ms high
#define CFG_FC_PIN14_DEFAULT                CFG_FC_PIN14_OFFLOGICLOW
extern const char * CFG_FC_PIN14[4];

#define CFG_FC_RX_CRC_OFF                   0 //0b00
#define CFG_FC_RX_CRC_ON                    1 //0b01
#define CFG_FC_RX_CRC_DEFAULT               CFG_FC_RX_CRC_OFF
extern const char * CFG_FC_RX_CRC[2];

#define CFG_FC_TX_CRC_OFF                   0 //0b00
#define CFG_FC_TX_CRC_ON                    1 //0b01
#define CFG_FC_TX_CRC_DEFAULT               CFG_FC_TX_CRC_OFF
extern const char * CFG_FC_TX_CRC[2];

#define CFG_FC_TELEMETRY_OFF                0 //0b00
#define CFG_FC_TELEMETRY_ON                 1 //0b01
#define CFG_FC_TELEMETRY_DEFAULT            CFG_FC_TELEMETRY_OFF
extern const char * CFG_FC_TELEMETRY[2];

#define CFG_FC_TELEMETRY_RATE_P10HZ         0 //0b00
#define CFG_FC_TELEMETRY_RATE_1HZ           1 //0b01
#define CFG_FC_TELEMETRY_RATE_2HZ           2 //0b10
#define CFG_FC_TELEMETRY_RATE_3HZ           3 //0b11
#define CFG_FC_TELEMETRY_RATE_DEFAULT       CFG_FC_TELEMETRY_RATE_P10HZ
extern const char * CFG_FC_TELEMETRY_RATE[4];

#define CFG_FC_TELEMETRY_DUMP_OFF           0 //0b00
#define CFG_FC_TELEMETRY_DUMP_ON            1 //0b01
#define CFG_FC_TELEMETRY_DUMP_DEFAULT       CFG_FC_TELEMETRY_DUMP_OFF
extern const char * CFG_FC_TELEMETRY_DUMP[2];

#define CFG_FC_BEACON_OA_COMMANDS_OFF       0 //0b00
#define CFG_FC_BEACON_OA_COMMANDS_ON        1 //0b01
#define CFG_FC_BEACON_OA_COMMANDS_DEFAULT   CFG_FC_BEACON_OA_COMMANDS_OFF
extern const char * CFG_FC_BEACON_OA_COMMANDS[2];

#define CFG_FC_BEACON_CODE_UPLOAD_OFF       0 //0b00
#define CFG_FC_BEACON_CODE_UPLOAD_ON        1 //0b01
#define CFG_FC_BEACON_CODE_UPLOAD_DEFAULT   CFG_FC_BEACON_CODE_UPLOAD_OFF
extern const char * CFG_FC_BEACON_CODE_UPLOAD[2];

#define CFG_FC_BEACON_CODE_RESET_OFF        0 //0b00
#define CFG_FC_BEACON_CODE_RESET_ON         1 //0b01
#define CFG_FC_BEACON_CODE_RESET_DEFAULT    CFG_FC_BEACON_CODE_RESET_OFF
extern const char * CFG_FC_BEACON_RESET[2];

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

#endif
