/*
 * =====================================================================================
 *
 *       Filename:  SC_he100.c
 *
 *    Description:  Library to implement He100 functionality. Build as static library.
 *
 *        Version:  1.0
 *        Created:  13-09-20 08:23:35 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Shawn Bulger (),
 *   Organization:  Space Concordia
 *
 * =====================================================================================
 */
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#include <stdlib.h>     /*  C Standard General Utilities Library */
#include <stdio.h>      /*  Standard input/output definitions */
#include <stdint.h>     /*  Standard integer types */
#include <string.h>     /*  String function definitions */
#include <unistd.h>     /*  UNIX standard function definitions */
#include <errno.h>      /*  Error number definitions */
#include <poll.h>       /*  Definitions for the poll() function */

// serial library from utls 
#include "SC_serial.h"

// TODO group and document these includes
#include "time.h"

// project includes
#include <SC_he100.h>   /*  Helium 100 header file */
//#include <he100.h>      /*  exposes the correct serial device location */
#include "fletcher.h"
#include <timer.h>
#include "SpaceDecl.h"
#include "shakespeare.h"

// logging 
#define PROCESS "HE100"
#define MAX_LOG_BUFFER_LEN CS1_MAX_LOG_ENTRY 

/**
 * Function to write a given byte sequence to the serial device
 * @param fdin - the file descriptor representing the serial device
 * @param bytes - the char array containing the byte sequence to write
 * @param size - the length of the array in bytes
 */
int
HE100_write (int fdin, unsigned char *bytes, size_t size)
{
    int write_return = 1; // return value 
    int w; // to count bytes written
    if (fdin!=0) w = write (fdin, bytes, size); // Write byte array
    else return HE_FAILED_OPEN_PORT;

    fflush( NULL ); fsync(fdin); // TODO fdin, is a tty device, ineffective?

    unsigned char response_buffer[MAX_FRAME_LENGTH]; // TODO this will not be used, fault of refactoring decision

    int valid_bytes_returned = 0;

    if (bytes[HE_CMD_BYTE] != CMD_GET_CONFIG) // some commands manually manage reading responses
    { // Issue a read to check for ACK/NOACK
        valid_bytes_returned = HE100_read(fdin, 2, response_buffer);
    }  else {
        valid_bytes_returned = 1;
    }
    if ( valid_bytes_returned >= 0 && w>0 ) {
       write_return = 0;
    }
    return write_return;
}

/**
 * Function to validate a given frame
 * @param response - the frame data to be validated
 * @param length - the entire length of the frame in bytes
 * @return int - 0 if valid, else error code
 */
int
HE100_validateFrame (unsigned char *response, size_t length)
{
    int r=1; // return value

    // Check first if command byte is a valid command
    if ( HE100_referenceByteSequence(&response[HE_CMD_BYTE],HE_CMD_BYTE) == HE_INVALID_COMMAND ) 
    {
        return HE_INVALID_COMMAND;
    } 
    else
    { 
        r = 0; // so far so good
    }

    // calculate positions for payload and checksums
    size_t data_length = length - 2; // response minus 2 sync bytes
    int pb1 = length-2; int pb2 = length-1;

    // calculate payload length
    size_t payload_length;
    if ( length >= 10 ) 
    { // a message of this minimum length will have a payload
        payload_length = length - WRAPPER_LENGTH; // response minus header minus 4 checksum bytes and 2 sync bytes and 2 length bytes
        // validate length
        if ( response[HE_LENGTH_BYTE] != payload_length ) return CS1_WRONG_LENGTH;    
    } 
    else 
    { // empty payload control sequences
        payload_length = 0;
    }

    // generate and compare header checksum
    fletcher_checksum h_chksum = fletcher_checksum16(response+2,4);
    uint8_t h_s1_chk = memcmp(&response[HE_HEADER_CHECKSUM_BYTE_1], &h_chksum.sum1, 1);
    uint8_t h_s2_chk = memcmp(&response[HE_HEADER_CHECKSUM_BYTE_2], &h_chksum.sum2, 1);
    int h_chk = h_s1_chk + h_s2_chk; // should be zero given valid chk
    
    int p_chk=0;
    uint8_t p_s1_chk;
    uint8_t p_s2_chk;
    fletcher_checksum p_chksum;
    if (payload_length > 0) {
        // generate and compare payload checksum
        p_chksum = fletcher_checksum16(response+2,data_length-2); // chksum everything except 'He' and payload checksum bytes
        p_s1_chk = memcmp(&response[pb1], &p_chksum.sum1, 1);
        p_s2_chk = memcmp(&response[pb2], &p_chksum.sum2, 1);
        p_chk = p_s1_chk + p_s2_chk; // should be zero given valid chk
    }

    if (response[HE_LENGTH_BYTE_0] == response[HE_LENGTH_BYTE] ) /* ACK or NOACK or EMPTY length */
    {
        char output[MAX_LOG_BUFFER_LEN];
        Shakespeare::Priority logPriority;
        if (response[4] == HE_ACK) {
            snprintf (output, MAX_LOG_BUFFER_LEN, "ACK>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::NOTICE;
            r = 0;
        } else if (response[4] == HE_NOACK) {
            snprintf (output, MAX_LOG_BUFFER_LEN, "NACK>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::ERROR;
            r = HE_FAILED_NACK;
        } else if (response[4] == 0) {
            snprintf (output, MAX_LOG_BUFFER_LEN, "Empty Response>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::ERROR;
            r = HE_EMPTY_RESPONSE;
        } else {
            snprintf (output, MAX_LOG_BUFFER_LEN, "Unknown byte sequence>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::ERROR;
            r = HE_INVALID_BYTE_SEQUENCE;
        }
        Shakespeare::log(logPriority, PROCESS, output);
    }
    
    if (h_chk != 0) {
       char error[MAX_LOG_BUFFER_LEN];
       sprintf (
                error, 
                "Invalid header checksum. Incoming: [%d,%d] Calculated: [%d,%d] %s, %d", 
                (uint8_t)response[HE_HEADER_CHECKSUM_BYTE_1],(uint8_t)response[HE_HEADER_CHECKSUM_BYTE_2],
                (uint8_t)h_chksum.sum1,(uint8_t)h_chksum.sum2, 
                __func__, __LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, error);
        r=HE_FAILED_CHECKSUM;
    }

    if (p_chk != 0) {
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (
                error, 
                "Invalid payload checksum. Incoming: [%d,%d] Calculated: [%d,%d] %s, %d", 
                (uint8_t)response[pb1],(uint8_t)response[pb2],(uint8_t)p_chksum.sum1,(uint8_t)p_chksum.sum2,
                __func__, __LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, error);
        r=HE_FAILED_CHECKSUM;
    }

    if (payload_length==0 && response[HE_LENGTH_BYTE] != 0) {
#ifdef CS1_DEBUG
        HE100_dumpHex(stdout, response, length);
#endif
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (
                error, 
                "Unrecognized byte sequence. Zero length payload should be indicated by zero length byte, or by ACK/NACK %s, %d", 
                __func__, __LINE__
        );
        Shakespeare::log(Shakespeare::WARNING, PROCESS, error);
    }

    return r;
}

/* Function to dump a given array to a given file descriptor */
int
HE100_dumpBinary(FILE *fdout, unsigned char *bytes, size_t size)
{
    fprintf(stdout,"\ndumping bytes\n");
    int ret_val;
    ret_val = fwrite (bytes, 1, size, fdout);
    return ret_val;
}

/* Function to dump a given array as hex to stdoout */
void
HE100_dumpHex(FILE *fdout, unsigned char *bytes, size_t size)
{
    size_t j=0;
    fprintf(fdout,"Dumping %d bytes: ", (unsigned int) size);
    for (j=0;j<size;j++)
    {
        fprintf(fdout,"%02X ",bytes[j]);
        //fprintf(fdout,"%s ",(char*)&bytes[j]);
    }
    fprintf(fdout,"\r\n");
    return;
}

void HE100_snprintfHex(char * output_hex_array, unsigned char * input_byte_array, size_t size)
{
    size_t raw_byte_index       = 0;
    size_t output_hex_index     = raw_byte_index; // both indicies start at 0
    size_t entry_length         = 3; // length of two hex digits plus a space

    for (raw_byte_index=0;raw_byte_index<size;raw_byte_index++)
    {
       output_hex_index=raw_byte_index*entry_length; // output_hex_index is 3 * input_byte_index 
       
       if (output_hex_index<MAX_LOG_BUFFER_LEN)
       {
           snprintf(
             output_hex_array+output_hex_index,
             entry_length,
             "%02X ",
             input_byte_array[raw_byte_index]
           );
       } 
       else 
       {
         break;
       }
    }
    
    return;
}


/**
 * The HE100_read function obtains communication payloads from the 
 * serial device and returns an execution status.
 *
 * It reads bytes in single-file from the serial device and
 * appends them to a reference array.
 *
 * A timer is set to poll a the file descriptor which 
 * handles the serial connection to the RX pin of the Radio. 
 * It does some preliminary parsing to identify the incoming frames,
 * and copies the payload data to a reference buffer if successful.
 * If not sucessful, it logs an error using Shakespeare
 *
 * Parameters:
 * payload - the buffered bytes returned parsed from the radio
 * fdin - file descriptor for serial communication
 * payload - a reference buffer to pass sucessfully parsed data
 *
 * return r 
 * - if successful, return the number of payload bytes recieved (total minus metabytes) 
 * - if not successful for any reason, return -1 
 *
 * Notes: 
 * this function is doing too much
 *  1 start timer
 *  2 poll
 *  3 looping read byte
 *   3a set breakpoint as appropriate 
 *  4 validate frame upon reaching breakpoint
 *   4a if valid: strip and pass payload data through reference buffer
 *   4b if invalid: log error and exit return -1
 * TODO: split step 4 into another function, test
 **/
int
HE100_read (int fdin, time_t read_time, unsigned char * payload)
{
    if (payload==NULL) return -1;

    // Read response
    unsigned char buffer[1]; // to hold each byte as device is read
    unsigned char response[MAX_FRAME_LENGTH] = {0}; // initialize empty response buffer
    int i=0;
    int r=-1; // return value for HE100_read
    int breakcond=MAX_FRAME_LENGTH;

    // start timer from call
    timer_t read_timer = timer_get();
    timer_start(&read_timer,read_time,0);

    // Variables for select
    int ret_value;

    struct pollfd fds;
    fds.fd = fdin;
    fds.events = POLLIN;
   
    if (fdin==0) return -1;

    // Read continuously from serial device
    while (!timer_complete(&read_timer))
    {
        if ( ( ret_value = poll(&fds, 1, 5) > -1 ) ) // if a byte is ready to be read
        {
            read(fdin, &buffer, 1);
            // set break condition based on incoming byte pattern
            if ( i==HE_LENGTH_BYTE_0 && (buffer[0] == 0x0A || buffer[0] == 0xFF) ) { 
                // TODO could this EVER also be a large length > 255? 
                // COULD BE getting an ack
                breakcond=8; // ack is 8 bytes
            } else if ( i==5 && breakcond==255 ) { 
                // this is the length byte, and we haven't already set the break point. Set break point accordingly
                breakcond = buffer[0] + WRAPPER_LENGTH;
            }
            // increment response array values based on byte pattern
            // if byte is referenced correctly, continue
            if ( HE100_referenceByteSequence(buffer,i) == 0 ) {
                response[i]=buffer[0];
                buffer[0] = '\0';
                i++;
            } else {
                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
            }
            if (i==breakcond) 
            {
                // TODO this section might be split into another function
                if (i>0) 
                {   // we are at the expected end of our message, time to validate
                    int SVR_result = HE100_validateFrame(response, breakcond);
                    if ( SVR_result == 0 ) 
                    {   // valid frame
                        r = 0; // TODO why is this set to zero here if only to be reset to payload_length?
                        size_t payload_length;
                        if (breakcond >= 10) {
                            payload_length = breakcond - WRAPPER_LENGTH;
                            memcpy (payload, response+HE_FIRST_PAYLOAD_BYTE, payload_length);
                        } else payload_length=0;
                        r=payload_length;
                    }
                    else if (SVR_result == CS1_NULL_MALLOC) 
                    {   // memory allocation problem in validateFrame()
                        char error[MAX_LOG_BUFFER_LEN];
                        snprintf (error, MAX_LOG_BUFFER_LEN, "Memory allocation problem: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
                        Shakespeare::log(Shakespeare::ERROR, PROCESS, error);
                        r=-1;
                    }
                    else
                    {   // something unexpected
                        char error[MAX_LOG_BUFFER_LEN];
                        snprintf (error, MAX_LOG_BUFFER_LEN, "Invalid Data: %d, %d, %s, %d", fdin, SVR_result, __func__, __LINE__);
                        Shakespeare::log(Shakespeare::ERROR, PROCESS, error);
                        r=-1;
                       
                        // soft reset the transceiver
                        char log_msg[MAX_LOG_BUFFER_LEN]; // prepare to log result of soft reset
                        if ( HE100_softReset(fdin) == 0 ) {
                            sprintf (log_msg, "Soft Reset written successfully!: %d, %d, %s, %d", fdin, SVR_result, __func__, __LINE__);
                        } 
                        else { 
                            sprintf (log_msg, "Soft Reset FAILED: %d, %d, %s, %d", fdin, SVR_result, __func__, __LINE__);
                        }
                        Shakespeare::log(Shakespeare::ERROR, PROCESS, log_msg);
                    }
                    break; // TODO why is this break needed?
                }
                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
            }
            buffer[0] = '\0'; // wipe buffer each time
        }
        else if (ret_value == -1) 
        {   // bad or no read
            char error[MAX_LOG_BUFFER_LEN];
            snprintf (error, MAX_LOG_BUFFER_LEN, "Problem with poll(): %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
            Shakespeare::log(Shakespeare::ERROR, PROCESS, error);
            r = -1;
        }
    }
    return r;
}

// @param prepared_transmission - the bytes prepared for use externally
// @return int - exit status
int 
HE100_prepareTransmission(unsigned char *payload, unsigned char *prepared_transmission, size_t length, unsigned char *command)
{
    int has_payload; // bool to define whether the transmission has payload or not

    // set the array bounds based on command
    if (command[1] == 0x01 || command[1] == 0x02 || command[1] == 0x12 || command[1] == 0x05) /* empty payload */ {
        has_payload = 0;
    } else {
        has_payload = 1;
    }

    // TODO how to better check if length is not accurate?
    /*
    size_t count,i; // integer to count and verify payload length
    count=0;
    for (i=0; i<MAX_FRAME_LENGTH; i++) {
        if (payload[i] == '\0') break;
        count++;
    }
    if (count != length) return CS1_WRONG_LENGTH; 
    */

    // attach sync bytes to final transmission byte array
    prepared_transmission[HE_SYNC_BYTE_1] = SYNC1;
    prepared_transmission[HE_SYNC_BYTE_2] = SYNC2;

    // attach command bytes to intermediary payload byte array
    prepared_transmission[HE_TX_RX_BYTE] = (unsigned char) command[0];
    prepared_transmission[HE_CMD_BYTE] = (unsigned char) command[1];

    // attach length bytes
    prepared_transmission[HE_LENGTH_BYTE_0] = length >> 8;
    prepared_transmission[HE_LENGTH_BYTE] = (unsigned char) length & 0xff;

    // generate and attach header checksum
    fletcher_checksum header_checksum = fletcher_checksum16(prepared_transmission+2,4);
    prepared_transmission[HE_HEADER_CHECKSUM_BYTE_1] = (unsigned char) header_checksum.sum1 & 0xff;
    prepared_transmission[HE_HEADER_CHECKSUM_BYTE_2] = (unsigned char) header_checksum.sum2 & 0xff;
   
    
    if( has_payload==1 ) // real payload
    {
        memcpy (prepared_transmission+HE_FIRST_PAYLOAD_BYTE,payload,length);
        // generate and attach payload checksum
        fletcher_checksum payload_checksum = fletcher_checksum16(prepared_transmission+HE_TX_RX_BYTE,length+6); // chksum everything except first two bytes 'He'
        prepared_transmission[HE_FIRST_PAYLOAD_BYTE+length] = payload_checksum.sum1;
        prepared_transmission[HE_FIRST_PAYLOAD_BYTE+length+1] = payload_checksum.sum2;
    }

    return 0;
}

/* Function to ensure byte-by-byte that we are receiving a HE100 frame */
int
HE100_referenceByteSequence(unsigned char *response, int position)
{
    int r = CS1_INVALID_BYTE_SEQUENCE;
    switch ((int)position)
    {
        case 0   : // first position should be 0x48 : 72
                if ((int)*response==72) r=0;
                break;
        case 1   : // second position should be 0x65 : 101
                if ((int)*response==101) r=0;
                break;
        case 2   : // response tx/rx command should be 0x20
                // if response == transmission, He100 device is off!
                if ((int)*response==16) {
                    fprintf(stderr,"HE100_read: He100 is off! Line:%d", __LINE__);
                    return HE_POWER_OFF;
                }
                if ((int)*response==32) r=0; // CMD_RECEIVE  0x20
                break;
        case 3   : // response command could be between 1-20
                if (*response > 0x00 && *response <= 0x20) { 
                  r=0;
                  //r=(int)*response; // wanted to return response, but breaks exit status convention
                } else {
                  r=HE_INVALID_COMMAND;
                } 
                break;
        case 4   : // first length byte
                if (
                       *response == 0x00 // start of length byte
                    || *response == 0x0A // start of ack, should match 5
                    || *response == 0xFF // start of noack, should match 5
                   ) r=0;
                break;
        //case 5   : // real length byte
                //if ((int)*response<MAX_FRAME_LENGTH) r=(int)*response;
                //break;
        default  : r=0; break;
    }
    return r;
}

int 
HE100_dispatchTransmission(int fdin, unsigned char *payload, size_t payload_length, unsigned char *command)
{
    // initialize transmission array to store prepared byte sequence
    unsigned char transmission[MAX_FRAME_LENGTH] = {0};
    memset (transmission,'\0',MAX_FRAME_LENGTH);    

    int prepare_result = HE100_prepareTransmission(payload,transmission,payload_length,command);
#ifdef CS1_DEBUG
      char debug_msg[MAX_LOG_BUFFER_LEN] = {0};
      char hex_representation[MAX_LOG_BUFFER_LEN] = {0};
      HE100_snprintfHex(hex_representation,transmission,payload_length+WRAPPER_LENGTH);      
      snprintf (debug_msg,MAX_LOG_BUFFER_LEN, "Prepared payload: %s",hex_representation);
#endif
    // if preparation successful, write the bytes to the radio
    if ( prepare_result == 0) {
#ifdef CS1_DEBUG
      Shakespeare::log(Shakespeare::NOTICE,PROCESS,"Prepare successful");
      Shakespeare::log(Shakespeare::NOTICE,PROCESS,debug_msg);
#endif
      return HE100_write(fdin,transmission,payload_length+WRAPPER_LENGTH);
    } else {
#ifdef CS1_DEBUG
      Shakespeare::log(Shakespeare::ERROR,PROCESS,"Prepare failed");
      Shakespeare::log(Shakespeare::ERROR,PROCESS,debug_msg);
#endif
        return 1;
    }
}

/**
 * Function to return NOOP byte sequence
 * no arguments
 */
int
HE100_NOOP (int fdin)
{
   unsigned char noop_payload[1] = {0};
   unsigned char noop_command[2] = {CMD_TRANSMIT, CMD_NOOP};
   return HE100_dispatchTransmission(fdin,noop_payload,0,noop_command);
}

/**
 * Function returning byte sequence to set the beacon message
 * unsigned char *beacon_message_payload message to transmit
 */
int
HE100_transmitData (int fdin, unsigned char *transmit_data_payload, size_t transmit_data_len)
{
    unsigned char transmit_data_command[2] = {CMD_TRANSMIT, CMD_TRANSMIT_DATA};
    return HE100_dispatchTransmission(fdin,transmit_data_payload,transmit_data_len,transmit_data_command);
}

/**
 * Function returning byte sequence to enable beacon on given interval
 * int beacon_interval interval in seconds
 */
int
HE100_setBeaconInterval (int fdin, int beacon_interval)
{
   if (beacon_interval > 255 ) return -1; // TODO not possible
   unsigned char beacon_interval_payload[1];
   beacon_interval_payload[0] = beacon_interval & 0xff;
   unsigned char beacon_interval_command[2] = {CMD_TRANSMIT, CMD_BEACON_CONFIG};
   return HE100_dispatchTransmission(fdin,beacon_interval_payload,1,beacon_interval_command);
}

/**
 * Function returning byte sequence to set the beacon message
 * unsigned char *beacon_message_payload message to transmit
 */
int
HE100_setBeaconMessage (int fdin, unsigned char *set_beacon_message_payload, size_t beacon_message_len)
{
   unsigned char set_beacon_message_command[2] = {CMD_TRANSMIT, CMD_BEACON_DATA};
   return HE100_dispatchTransmission(fdin,set_beacon_message_payload,beacon_message_len,set_beacon_message_command);
}

/**
 * Function returning byte sequence to amplify power based
 * on input int power_level
 * int power_level decimal value from 0-255 (0%-100%)
 */
int
HE100_fastSetPA (int fdin, int power_level)
{
   unsigned char fast_set_pa_payload[1];
   fast_set_pa_payload[0] = power_level & 0xff;
   if (power_level > MAX_POWER_LEVEL || power_level < MIN_POWER_LEVEL) {
     return 1;
   } 
   unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
   return HE100_dispatchTransmission(fdin,fast_set_pa_payload,1,fast_set_pa_command);
}

/**
 * Function returning byte sequence to soft reset HE100 board and restore flash settings
 * no arguments
 */
int
HE100_softReset(int fdin)
{
   unsigned char soft_reset_payload[1] = {0};
   unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
   return HE100_dispatchTransmission(fdin,soft_reset_payload,0,soft_reset_command);
}

/**
 * Function returning byte sequence to return firmware version
 * no arguments
 */
int
HE100_readFirmwareRevision(int fdin)
{
   unsigned char read_firmware_revision_payload[1] = {0};
   unsigned char read_firmware_revision_command[2] = {CMD_TRANSMIT, CMD_READ_FIRMWARE_V};
   return HE100_dispatchTransmission(fdin,read_firmware_revision_payload,0,read_firmware_revision_command);
}

/**
 * Function to parse the results from a character array into
 * a he100_settings struct
 */
struct he100_settings
HE100_collectConfig (unsigned char * buffer)
{
    he100_settings settings;
    settings.interface_baud_rate = buffer[CFG_IF_BAUD_BYTE];
    settings.tx_power_amp_level = buffer[CFG_PA_BYTE];
    settings.rx_rf_baud_rate = buffer[CFG_RF_RX_BAUD_BYTE];
    settings.tx_rf_baud_rate = buffer[CFG_RF_TX_BAUD_BYTE];
    settings.rx_modulation = buffer[CFG_RX_MOD_BYTE];
    settings.tx_modulation = buffer[CFG_TX_MOD_BYTE];

    // swap endianess
    // TODO CONDITIONAL ENDIANNESS conversion
    settings.rx_freq =
          buffer[CFG_RX_FREQ_BYTE4] << 24 |
          buffer[CFG_RX_FREQ_BYTE3] << 16 |
          buffer[CFG_RX_FREQ_BYTE2] << 8  |
          buffer[CFG_RX_FREQ_BYTE1]
    ;
    settings.tx_freq =
          buffer[CFG_TX_FREQ_BYTE4] << 24 |
          buffer[CFG_TX_FREQ_BYTE3] << 16 |
          buffer[CFG_TX_FREQ_BYTE2] << 8  |
          buffer[CFG_TX_FREQ_BYTE1]
    ;

    memcpy(
            settings.source_callsign,
            (unsigned char*)buffer+CFG_SRC_CALL_BYTE,
            CFG_CALLSIGN_LEN
    );

    memcpy(
            settings.destination_callsign,
            (unsigned char*)buffer+CFG_DST_CALL_BYTE,
            CFG_CALLSIGN_LEN
    );

    settings.tx_preamble =
        buffer[CFG_TX_PREAM_BYTE] << 8 |
        buffer[CFG_TX_PREAM_BYTE+1]
    ;

    settings.tx_postamble =
        buffer[CFG_TX_POSTAM_BYTE] << 8 |
        buffer[CFG_TX_POSTAM_BYTE+1]
    ;

    memcpy (
            &settings.function_config,
            (unsigned char*)buffer+CFG_FUNCTION_CONFIG_BYTE,
            sizeof(struct function_config)
            // CFG_FUNCTION_CONFIG_LENGTH
    );

    memcpy (
            &settings.function_config2,
            (unsigned char*)buffer+CFG_FUNCTION_CONFIG2_BYTE,
            sizeof(struct function_config2)
            // CFG_FUNCTION_CONFIG2_LENGTH
    );

    settings.ext_conf_setting = buffer[CFG_EXT_BYTE];

    //memcpy (&settings,buffer+WRAPPER_LENGTH,CFG_PAYLOAD_LENGTH); // copies char buf into struct
    return settings;
}

/**
 * This function parses and prints out a the configuration passed by
 * a he100_settings struct to a given file pointer
 */
void 
HE100_printSettings( FILE* fdout, struct he100_settings settings ) {
    fprintf(fdout,"Interface Baud Rate:   %s [%d]\n\r", if_baudrate[settings.interface_baud_rate],settings.interface_baud_rate);
    fprintf(fdout,"TX Power Amp Level:    %d [%d] \r\n", settings.tx_power_amp_level*100/255,settings.tx_power_amp_level);
    fprintf(fdout,"RX Baud Rate:          %s [%d] \r\n", rf_baudrate[settings.rx_rf_baud_rate],settings.rx_rf_baud_rate);
    fprintf(fdout,"TX Baud Rate:          %s [%d] \r\n", rf_baudrate[settings.tx_rf_baud_rate],settings.tx_rf_baud_rate);
    fprintf(fdout,"RX Frequency:          %d \r\n", settings.rx_freq);
    fprintf(fdout,"TX Frequency:          %d \r\n", settings.tx_freq);

    struct function_config fc1 = settings.function_config;

    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_LED[fc1.crc_rx],fc1.crc_rx);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_PIN13[fc1.pin13],fc1.pin13);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_PIN14[fc1.pin14],fc1.pin14);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_RX_CRC[fc1.crc_rx],fc1.crc_rx);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_TX_CRC[fc1.crc_tx],fc1.crc_tx);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_TELEMETRY[fc1.telemetry_status],fc1.telemetry_status);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_TELEMETRY_RATE[fc1.telemetry_rate],fc1.telemetry_rate);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_TELEMETRY_DUMP[fc1.telemetry_dump_status],fc1.telemetry_dump_status);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_BEACON_OA_COMMANDS[fc1.beacon_oa_cmd_status],fc1.beacon_oa_cmd_status);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_BEACON_CODE_UPLOAD[fc1.beacon_code_upload_status],fc1.beacon_code_upload_status);
    fprintf(fdout,"%s [%02X] \r\n",CFG_FC_BEACON_RESET[fc1.beacon_radio_reset_status],fc1.beacon_radio_reset_status);

    fprintf(fdout,"Source Callsign:       %s \r\n", settings.source_callsign);
    fprintf(fdout,"Destination Callsign:  %s \r\n", settings.destination_callsign);
    fprintf(fdout,"TX Preamble:           %d \r\n", settings.tx_preamble);
    fprintf(fdout,"TX Postamble:          %d \r\n", settings.tx_postamble);

    char ext_conf_value[32] = {0}; 
    // validate EXT functions
    switch (settings.ext_conf_setting)
    {
        case 0 :
           sprintf(ext_conf_value, "All Functions off");
           break; 
        case CFG_EXT_PING_ON:
           sprintf(ext_conf_value, "PING ON");
           break;
        case CFG_EXT_CODEUPLOAD_ON:
           sprintf(ext_conf_value, "CODEUPLOAD ON");
           break; 
        case CFG_EXT_RESET_ON:
           sprintf(ext_conf_value, "RESET ON");
           break; 
        default : 
           sprintf(ext_conf_value, "INVALID SETTING");
           break;
    }
    fprintf(fdout,"EXT:                   %s [%02X] \r\n", ext_conf_value, settings.ext_conf_setting);
   
    struct function_config2 fc2 = settings.function_config2;
    fprintf(
        fdout,
        "Function Config2       [%02X][%02X][%02X][%02X]\r\n",
        fc2.rafc,fc2.rxcw,fc2.txcw,fc2.tbd
    );
}

/**  
 * This function prepares the byte sequence parsed from a 
 * he100_settings struct to be passed over the serial interface
 * to the transceiver
 **/
int 
HE100_prepareConfig (unsigned char * prepared_bytes, struct he100_settings settings) {

    // TODO should this be changed with the serial connection as well?
    memcpy(
        &prepared_bytes[CFG_IF_BAUD_BYTE],
        (unsigned char*)&settings.interface_baud_rate,
        sizeof(settings.interface_baud_rate)
    );
    memcpy(
        &prepared_bytes[CFG_PA_BYTE],
        (unsigned char*)&settings.tx_power_amp_level,
        sizeof(settings.tx_power_amp_level)
    );
    memcpy(
        &prepared_bytes[CFG_RF_RX_BAUD_BYTE],
        (unsigned char*)&settings.rx_rf_baud_rate,
        sizeof(settings.rx_rf_baud_rate)
    );
    memcpy(
        &prepared_bytes[CFG_RF_TX_BAUD_BYTE],
        &settings.tx_rf_baud_rate,
        sizeof(settings.tx_rf_baud_rate)
    );
    memcpy(
        &prepared_bytes[CFG_RX_MOD_BYTE],
        &settings.rx_modulation,
        sizeof(settings.rx_modulation)
    );
    memcpy(
        &prepared_bytes[CFG_TX_MOD_BYTE],
        &settings.tx_modulation,
        sizeof(settings.tx_modulation)
    );

    unsigned char * rx_freq = (unsigned char *)&settings.rx_freq;
    uint32_t big_endian_rx_freq = 
          rx_freq[3] << 24 |
          rx_freq[2] << 16 |
          rx_freq[1] << 8  |
          rx_freq[0]  
    ;
    memcpy(
        &prepared_bytes[CFG_RX_FREQ_BYTE1],
        &big_endian_rx_freq,
        sizeof(settings.rx_freq)
    );
    unsigned char * tx_freq = (unsigned char *)&settings.tx_freq;
    uint32_t big_endian_tx_freq =
          tx_freq[3] << 24 |
          tx_freq[2] << 16 |
          tx_freq[1] << 8  |
          tx_freq[0]  
    ;
    memcpy(
        &prepared_bytes[CFG_TX_FREQ_BYTE1],
        &big_endian_tx_freq,
        sizeof(settings.tx_freq)
    );

    memcpy(
        &prepared_bytes[CFG_SRC_CALL_BYTE],
        settings.source_callsign,
        CFG_CALLSIGN_LEN
    );
    memcpy(
        &prepared_bytes[CFG_DST_CALL_BYTE],
        &settings.destination_callsign,
        CFG_CALLSIGN_LEN
    );

    unsigned char * tx_preamble = (unsigned char *)&settings.tx_preamble;
    uint16_t big_endian_tx_preamble = 
        tx_preamble[0] << 8 | 
        tx_preamble[1]
    ; 
    memcpy(
        &prepared_bytes[CFG_TX_PREAM_BYTE],
        &big_endian_tx_preamble,
        sizeof(settings.tx_preamble)
    );

    unsigned char * tx_postamble = (unsigned char *)&settings.tx_postamble;
    uint16_t big_endian_tx_postamble = 
        tx_postamble[0] << 8 |
        tx_postamble[1]
    ; 
    memcpy(
        &prepared_bytes[CFG_TX_POSTAM_BYTE],
        &big_endian_tx_postamble,
        sizeof(settings.tx_postamble)
    );

    memcpy(
        &prepared_bytes[CFG_FUNCTION_CONFIG_BYTE],
        &settings.function_config,
        CFG_FUNCTION_CONFIG_LENGTH
    );
    memcpy(
        &prepared_bytes[CFG_FUNCTION_CONFIG2_BYTE],
        &settings.function_config2,
        CFG_FUNCTION_CONFIG2_LENGTH
    );

    return HE_SUCCESS;
}

/** 
 * This function calls the dispatch to read the configuration
 * from the transceiver
 **/
int 
HE100_getConfig (int fdin, struct he100_settings * settings)
{
    int result = 1;
    unsigned char get_config_payload[1] = {0};
    unsigned char get_config_command[2] = {CMD_TRANSMIT, CMD_GET_CONFIG};
  
    // the response to this call will contain the  
    int get_config_result = HE100_dispatchTransmission(fdin,get_config_payload,0,get_config_command);

    if(get_config_result == 0)
    {
       unsigned char config_transmission[MAX_FRAME_LENGTH];
       if ( HE100_read(fdin, 2, config_transmission) > 0 ) // valid number of bytes is great than zero
       {
         // TODO verify this is a read frame!
         unsigned char config_bytes[CFG_PAYLOAD_LENGTH];
         memcpy (&config_bytes, config_transmission,CFG_PAYLOAD_LENGTH);
         *settings = HE100_collectConfig(config_bytes); 
         result = HE100_validateConfig(*settings); 
       } 
       else 
       {
         result = HE_FAILED_READ;
       } 
    }
    return result;
}

/**  
 *  This function validates a he100_settings struct
 *  and converts to char array for safe deployment
 **/ 
int 
HE100_validateConfig (struct he100_settings he100_new_settings)
{
    char validation_log_entry[MAX_LOG_BUFFER_LEN];

    // validate new interface baud rate setting
    if (
            he100_new_settings.interface_baud_rate >= MAX_IF_BAUD_RATE 
        //&&  he100_new_settings.interface_baud_rate <= MIN_IF_BAUD_RATE // always true
        // TODO WHY // &&  he100_new_settings.interface_baud_rate != CFG_DEF_IF_BAUD 
    ) 
    {
        sprintf(validation_log_entry,"%s %s ^%s@%d",
                HE_STATUS[HE_INVALID_IF_BAUD_RATE],if_baudrate[he100_new_settings.interface_baud_rate],
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_IF_BAUD_RATE;
    } 
    

    // validate new power amplification level
    if (
               he100_new_settings.tx_power_amp_level >= MAX_PA_LEVEL
            && he100_new_settings.tx_power_amp_level <= MIN_PA_LEVEL // always true
    ) 
    { 
        sprintf(validation_log_entry,"%s BAUD:%d ^%s@%d",
                HE_STATUS[HE_INVALID_POWER_AMP_LEVEL],he100_new_settings.tx_power_amp_level,
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_POWER_AMP_LEVEL;
    };

    // validate new rf baud rates
    if (
            he100_new_settings.rx_rf_baud_rate >= MAX_RF_BAUD_RATE
        &&  he100_new_settings.rx_rf_baud_rate <= MIN_RF_BAUD_RATE // always true
        &&  he100_new_settings.tx_rf_baud_rate >= MAX_RF_BAUD_RATE
        &&  he100_new_settings.tx_rf_baud_rate <= MIN_RF_BAUD_RATE // always true
    ) 
    {
        sprintf(validation_log_entry,"%s MAX:%d MIN:%d INDEX:%d BAUD:%s ^%s@%d",
                HE_STATUS[HE_INVALID_RF_BAUD_RATE],
                MAX_RF_BAUD_RATE, MIN_RF_BAUD_RATE,
                he100_new_settings.tx_power_amp_level,rf_baudrate[he100_new_settings.tx_power_amp_level],
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_RF_BAUD_RATE;
    }
   
    // validate modulation (USE DEFAULTS)
    if (
            he100_new_settings.rx_modulation != CFG_RX_MOD_DEFAULT
    )
    {
        sprintf(validation_log_entry,"%s [rx] expected[%d] actual[%d] ^%s@%d",
                HE_STATUS[HE_INVALID_RX_MOD],
                CFG_RX_MOD_DEFAULT,
                he100_new_settings.rx_modulation,
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_RX_MOD; 
    }

    if (
         he100_new_settings.tx_modulation != CFG_TX_MOD_DEFAULT        
    )
    {
        sprintf(validation_log_entry,"%s expected[%d] actual[%d] ^%s@%d",
                HE_STATUS[HE_INVALID_TX_MOD],
                CFG_TX_MOD_DEFAULT,
                he100_new_settings.tx_modulation,
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_TX_MOD;
    }

    // validate new RX setting
    if ( 
        (he100_new_settings.rx_freq <= MIN_UPPER_FREQ && he100_new_settings.rx_freq >= MAX_UPPER_FREQ)
     || (he100_new_settings.rx_freq <= MIN_LOWER_FREQ && he100_new_settings.rx_freq >= MAX_LOWER_FREQ)
    )
    { 
        sprintf(validation_log_entry,"%s rx:%d ^%s@%d",
                HE_STATUS[HE_INVALID_RX_FREQ],he100_new_settings.rx_freq,
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_RX_FREQ;
    }

    // validate new TX setting
    if ( 
        (he100_new_settings.tx_freq <= MIN_UPPER_FREQ && he100_new_settings.tx_freq >= MAX_UPPER_FREQ)
     || (he100_new_settings.tx_freq <= MIN_LOWER_FREQ && he100_new_settings.tx_freq >= MAX_LOWER_FREQ)
    )
    {
        sprintf(validation_log_entry,"%s tx:%d ^%s@%d",
                HE_STATUS[HE_INVALID_TX_FREQ],he100_new_settings.tx_freq,
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_TX_FREQ;
    }

    // validate callsigns (USE DEFAULTS) 
    if ( //1
        // length should be 6, valid CALLSIGN
        ( memcmp(he100_new_settings.source_callsign, CFG_SRC_CALL_DEF, 1 ) != 0 )
     || ( memcmp(he100_new_settings.destination_callsign, CFG_DST_CALL_DEF, 1 ) != 0 )
    )
    {
        sprintf(validation_log_entry,"%s source:%s destination:%s ^%s@%d",
                HE_STATUS[HE_INVALID_CALLSIGN],he100_new_settings.source_callsign,he100_new_settings.destination_callsign,
                __func__,__LINE__
        );
        Shakespeare::log(Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_CALLSIGN;
    }
  
    // validate TX Test CW (USE DEFAULTS) 

    // validate EXT functions
    switch (he100_new_settings.ext_conf_setting)
    {
        case 0 : /* all EXT functions off */
           break; 
        case CFG_EXT_PING_ON: /* ping on */        
           break;
        case CFG_EXT_CODEUPLOAD_ON: /* code upload on */ 
           break; 
        case CFG_EXT_RESET_ON: /* reset on */
           break; 
        default : 
           return HE_INVALID_EXT;
           break;
    } // if no valid option, go with default

    return 0;
}

/**
 *  Function to persist a given he100_settings struct after it passes validation
 */   
int 
HE100_setConfig (int fdin, struct he100_settings he100_new_settings)
{
    unsigned char set_config_payload[CFG_PAYLOAD_LENGTH] = {0}; 
    unsigned char set_config_command[2] = {CMD_TRANSMIT, CMD_SET_CONFIG};

    int validate_result=1, prepare_result=1;
    validate_result = HE100_validateConfig(he100_new_settings);

    if (validate_result==HE_SUCCESS) {
        prepare_result = HE100_prepareConfig(set_config_payload,he100_new_settings);
    }

    if (prepare_result == HE_SUCCESS) {
        // we have valid array
        return HE100_dispatchTransmission(fdin, set_config_payload, CFG_PAYLOAD_LENGTH, set_config_command);
    } else {
        return HE_INVALID_CONFIG;
    }
}

