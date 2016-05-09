/*
 * =====================================================================================
 *
 *       Filename:  SC_he100-translations.cpp
 *
 *    Description:  : Translational char arrays to convert status IDs and other informa-
 *                  tion into human readable output. IDs are defined in SC_he100.h
 *
 *        Version:  1.0
 *        Created:  16/12/2014 03:05:09 PM
 *       Revision:  none
 *       Compiler:  g++/gcc
 *
 *         Author:  Shawn
 *   Organization:  Space Concordia ConSat1
 *
 * =====================================================================================
 */

const char *HE_STATUS[34] = {
    "HE_SUCCESS",
    "HE_FAILED_OPEN_PORT",
    "HE_FAILED_CLOSE_PORT",
    "HE_NOT_A_TTY",
    "HE_INVALID_COMMAND",
    "HE_NOT_READY",
    "HE_POWER_OFF",
    "HE_FAILED_TTY_CONFIG",
    "HE_FAILED_SET_BAUD",
    "HE_FAILED_FLUSH",
    "HE_FAILED_CHECKSUM",
    "HE_FAILED_NACK",
    "HE_INVALID_BYTE_SEQUENCE",
    "HE_EMPTY_RESPONSE",
    "HE_INVALID_POWER_AMP_LEVEL",
    "HE_INVALID_IF_BAUD_RATE",
    "HE_INVALID_RF_BAUD_RATE",
    "HE_INVALID_RX_MOD",
    "HE_INVALID_TX_MOD",
    "HE_INVALID_RX_FREQ",
    "HE_INVALID_TX_FREQ",
    "HE_INVALID_CALLSIGN",
    "HE_INVALID_TX_PREAM",
    "HE_INVALID_TX_POSTAM",
    "HE_INVALID_RX_PREAM",
    "HE_INVALID_RX_POSTAM",
    "HE_INVALID_CRC",
    "HE_INVALID_DIO_PIN13",
    "HE_INVALID_RXTX_TEST",
    "HE_INVALID_EXT",
    "HE_INVALID_LED",
    "HE_INVALID_CONFIG",
    "HE_FAILED_GET_CONFIG",
    "HE_FAILED_READ"
};

const char *CMD_CODE_LIST[32] = {
    "CMD_NONE",             // 0x00 
    "CMD_NOOP",             // 0x01 
    "CMD_RESET",            // 0x02
    "CMD_TRANSMIT_DATA",    // 0x03
    "CMD_RECEIVE_DATA",     // 0x04
    "CMD_GET_CONFIG",       // 0x05
    "CMD_SET_CONFIG",       // 0x06
    "CMD_TELEMETRY",        // 0x07
    "CMD_WRITE_FLASH",      // 0x08
    "CMD_RF_CONFIGURE",     // 0x09
    "N/A",                  // 0x0a
    "N/A",                  // 0x0b
    "N/A",                  // 0x0c
    "N/A",                  // 0x0d
    "N/A",                  // 0x0e
    "N/A",                  // 0x0f
    "CMD_BEACON_DATA",      // 0x10
    "CMD_BEACON_CONFIG",    // 0x11
    "CMD_READ_FIRMWARE_V",  // 0x12
    "CMD_DIO_KEY_WRITE",    // 0x13
    "CMD_FIRMWARE_UPDATE",  // 0x14
    "CMD_FIRMWARE_PACKET",  // 0x15
    "CMD_FAST_SET_PA"       // 0x20
};

const char *if_baudrate[6] = {
    "9600","19200","38400","76800","115200"
};

const char *rf_baudrate[5] = {
    "1200","9600","19200","38400"
};

const char * CFG_FC_LED[4] = {
    "LED:                   OFFLOGICLOW",
    "LED:                   PULSE",
    "LED:                   TX_TOGGLE",
    "LED:                   RX_TOGGLE"
};

const char * CFG_FC_PIN13[4] = {
    "PIN13:                 OFFLOGICLOW",
    "PIN13:                 TX/RX Switch",
    "PIN13:                 2.5 Hx WDT",
    "PIN13:                 RX Packet Toggle"
};

const char * CFG_FC_PIN14[4] = {
    "PIN14:                 OFFLOGICLOW",
    "PIN14:                 DIO Over Air ON",
    "PIN14:                 DIO Over Air Pattern A (latching high)",
    "PIN14:                 DIO Over Air Pattern B (toggle, 72 ms high)"
};

const char * CFG_FC_RX_CRC[2] = {
    "RXCRC:                 OFF",
    "RXCRC:                 ON"
};

const char * CFG_FC_TX_CRC[2] = {
    "TXCRC:                 OFF",
    "TXCRC:                 ON"
};

const char * CFG_FC_TELEMETRY[2] = {
    "TELEMETRY:             OFF",
    "TELEMETRY:             ON"
};

const char * CFG_FC_TELEMETRY_RATE[4] = {
    "TELEMETRY RATE:        1/10 Hz",
    "TELEMETRY RATE:        1 Hz",
    "TELEMETRY RATE:        2 Hz",
    "TELEMETRY RATE:        3 Hz"
};

const char * CFG_FC_TELEMETRY_DUMP[2] = {
    "TELEMETRY DUMP:        OFF",
    "TELEMETRY DUMP:        ON"
};

const char * CFG_FC_BEACON_OA_COMMANDS[2] = { 
    "BEACON OA COMMANDS:    OFF",
    "BEACON OA COMMANDS:    ON"
};

const char * CFG_FC_BEACON_CODE_UPLOAD[2] = {
    "CODE UPLOAD:           OFF",
    "CODE UPLOAD:           ON"
};

const char * CFG_FC_BEACON_RESET[2] = {
    "CODE RESET:            OFF",
    "CODE RESET:            ON"
};

const char * CFG_BYTE_LIST[34] = {
    "CFG_IF_BAUD_BYTE          ",
    "CFG_PA_BYTE               ",
    "CFG_RF_RX_BAUD_BYTE       ",
    "CFG_RF_TX_BAUD_BYTE       ",
    "CFG_RX_MOD_BYTE           ",
    "CFG_TX_MOD_BYTE           ",
    "CFG_RX_FREQ_BYTE1         ",
    "CFG_RX_FREQ_BYTE2         ",
    "CFG_RX_FREQ_BYTE3         ",
    "CFG_RX_FREQ_BYTE4         ",
    "CFG_TX_FREQ_BYTE1         ", 
    "CFG_TX_FREQ_BYTE2         ", 
    "CFG_TX_FREQ_BYTE3         ", 
    "CFG_TX_FREQ_BYTE4         ", 
    "CFG_SRC_CALL_BYTE1        ",
    "CFG_SRC_CALL_BYTE2        ",
    "CFG_SRC_CALL_BYTE3        ",
    "CFG_SRC_CALL_BYTE4        ",
    "CFG_SRC_CALL_BYTE5        ",
    "CFG_SRC_CALL_BYTE6        ",
    "CFG_DST_CALL_BYTE1        ",
    "CFG_DST_CALL_BYTE2        ",
    "CFG_DST_CALL_BYTE3        ",
    "CFG_DST_CALL_BYTE4        ",
    "CFG_DST_CALL_BYTE5        ",
    "CFG_DST_CALL_BYTE6        ",
    "CFG_TX_PREAM_BYTE1        ",
    "CFG_TX_PREAM_BYTE2        ",
    "CFG_TX_POSTAM_BYTE1       ",
    "CFG_TX_POSTAM_BYTE2       ",
    "CFG_FUNCTION_CONFIG_BYTE1 ",
    "CFG_FUNCTION_CONFIG_BYTE2 ",
    "CFG_FUNCTION_CONFIG2_BYTE1",
    "CFG_FUNCTION_CONFIG2_BYTE2"
};
