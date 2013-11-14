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
 *         Author:  SHAWN BULGER (), 
 *   Organization:  
 *
 * =====================================================================================
 */

FILE *fdlog; // library log file 
FILE *fdata; // pipe to send valid payloads for external use

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

/* Function to apply configuration to HE100 on configured serial port address */
void HE100_configureInterface (int);

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
struct HE100_checksum HE100_fletcher16 (char *data, size_t bytes);

/**
 * Function to parse a given frame, validate it, and write its payload to pipe 
 * @param response - the frame data to be validated 
 * @param length - the entire length of the frame in bytes
 */
int HE100_validateResponse (char *response, size_t length);

/* Function to dump a given array to a given file descriptor */
int HE100_dumpBytes (FILE *fdout, unsigned char *bytes, size_t size);

/** Provide signal handling for HE100_read **/
//volatile sig_atomic_t stop;
//void inthand (int signum) { stop = 1; }

/**
 * Function to read bytes in single-file from the serial device and 
 * append them to and return a response array
 * 
 * @param fdin - the file descriptor representing the serial device
 */
int HE100_read (int fdin, time_t timeout);

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of data stream
 */
unsigned char * HE100_prepareTransmission (unsigned char *payload, size_t length, unsigned char *command);

/* Function to ensure byte-by-byte that we are receiving a HE100 frame */
//int HE100_referenceByteSequence(unsigned char *response, int position);
        
/** 
 * Function to decode validated and extracted data from response
 * @param response - the response data to interpret
 * @param length - the length of the data in bytes
 */
int HE100_interpretResponse (char *response, size_t length);

/**
 * Function to return NOOP byte sequence 
 * no arguments 
 */
unsigned char * HE100_NOOP();

/**
 * Function returning byte sequence to set the beacon message 
 * unsigned char *beacon_message_payload message to transmit 
 */
unsigned char * HE100_transmitData (unsigned char *transmit_data_payload, size_t transmit_data_len);

/**
 * Function returning byte sequence to enable beacon on given interval 
 * int beacon_interval interval in seconds 
 */
unsigned char * HE100_setBeaconInterval (int);

/**
 * Function returning byte sequence to set the beacon message 
 * unsigned char *beacon_message_payload message to transmit 
 */
unsigned char * HE100_setBeaconMessage (unsigned char *set_beacon_message_payload, size_t beacon_message_len);

/**
 * Function returning byte sequence to amplify power based
 * on input int power_level
 * int power_level decimal value from 0-255 (0%-100%)
 */
unsigned char * HE100_fastSetPA (int);

/**
 * Function returning byte sequence to soft reset HE100 board and restore flash settings 
 * no arguments
 */
unsigned char * HE100_softReset();

/**
 * Function returning byte sequence to return firmware version 
 * no arguments
 */
unsigned char * HE100_readFirmwareRevision();
