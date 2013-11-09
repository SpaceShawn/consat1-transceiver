/*
 * =====================================================================================
 *
 *       Filename:  he100.h
 *
 *    Description:  Header file for he100 library
 *
 *        Version:  1.0
 *        Created:  13-11-09 01:14:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

struct he100_settings {
  int 	interface_baud_rate; // Radio Interface Baud Rate (9600=0x00)
  int 	tx_power_amp_level; // Tx Power Amp Level (min=0x00, max=0xFF)
  int 	rx_rf_baud_rate; // Radio RX RF Baud Rate (9600=0x00)
  int 	tx_rf_baud_rate; // Radio TX RF Baud Rate (9600=0x00)
  int 	rx_modulation; // (0x00 = GFSK)
  int 	tx_modulation; // (0x00 = GFSK)
  int 	rx_freq; // Channel Tx Frequency 144200
  int 	tx_freq; // Channel Tx Frequency 431000
  char	source_callsign; // VA3ORB, default NOCALL
  char	destination_callsign; // VE2CUA, default CQ
  int	tx_preamble; // AX25 Mode Tx Preamble byte length (0x00 = 20 flags)
  int	tx_postamble; // AX25 Mode Tx Postamble byte length (0x00 = 20 flags)
  int function_config; // Radio Configuration discrete behaviors
  int function_config2; // Radio Configuration discrete behaviors #2
};

/* Function to open HE100 device on configured seial port address */
int SC_openPort(void);

/* Function to close serial device connection at given file descriptor */
int SC_closePort(int)

/* Function to apply configuration to HE100 on configured serial port address */
void SC_configureInterface (int);

/* Function to write a char array to a serial device at given file descriptor */
int SC_write(int, unsigned char, size_t);

/**
 * struct to hold values of fletcher checksum
 */
typedef struct SC_checksum {
    uint8_t sum1;
    uint8_t sum2;
} SC_checksum;

/* Optimized Fletcher Checksum */
struct SC_checksum SC_fletcher16 (char, size_t);

/**
 * Function to parse a given frame, validate it, and return its data
 * @param response - the frame data to be validated 
 * @param length - the entire length of the frame in bytes
 */
int SC_validateResponse (char, size_t);

/* Funcion to dump a given array to a given file descriptor */
int SC_dumpBytes(unsigned char, size_t);

/** Provide signal handling for SC_read **/
//volatile sig_atomic_t stop;
//void inthand (int signum) { stop = 1; }

/**
 * Function to read bytes in single-file from the serial device and 
 * append them to and return a response array
 * @param fdin - the file descriptor representing the serial device
 */
void SC_read (int fdin);

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of data stream
 */
unsigned char* SC_prepareTransmission(unsigned char, size_t, unsigned char);

/* Function to ensure byte-by-byte that we are receiving a HE100 frame */
int SC_referenceByteSequence(unsigned char, int);

/** 
 * Function to decode validated and extracted data from response
 * @param response - the response data to interpret
 * @param length - the length of the data in bytes
 */
int
SC_interpretResponse (char, size_t);

/**
 * Function to return NOOP byte sequence 
 * no arguments 
 */
unsigned char *SC_NOOP();

/**
 * Function returning byte sequence to set the beacon message 
 * unsigned char *beacon_message_payload message to transmit 
 */
unsigned char *SC_transmitData (unsigned char, size_t);

/**
 * Function returning byte sequence to enable beacon on given interval 
 * int beacon_interval interval in seconds 
 */
unsigned char *SC_setBeaconInterval(int);

/**
 * Function returning byte sequence to set the beacon message 
 * unsigned char *beacon_message_payload message to transmit 
 */
unsigned char *SC_setBeaconMessage (unsigned char, size_t);

/**
 * Function returning byte sequence to amplify power based
 * on input int power_level
 * int power_level decimal value from 0-255 (0%-100%)
 */
unsigned char * SC_fastSetPA (int);

/**
 * Function returning byte sequence to soft reset HE100 board and restore flash settings 
 * no arguments
 */
unsigned char *SC_softReset();

/**
 * Function returning byte sequence to return firmware version 
 * no arguments
 */
unsigned char *SC_readFirmwareRevision();
