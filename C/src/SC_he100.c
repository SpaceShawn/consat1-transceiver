/*
 * =====================================================================================
 *
 *       Filename:  SC_he100.c
 *
 *    Description:  Library to expose He100 functionality. Build as static library.
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
#include <stdlib.h>
#include <stdio.h>      /*  Standard input/output definitions */
#include <stdint.h>     /*  Standard integer types */
#include <string.h>     /*  String function definitions */
#include <unistd.h>     /*  UNIX standard function definitions */
#include <signal.h>     /*  Signal handling */
#include <fcntl.h>      /*  File control definitions */
#include <errno.h>      /*  Error number definitions */
#include <termios.h>    /*  POSIX terminal control definitions */
#include <SC_he100.h>   /*  Helium 100 header file */
#include "time.h"
#include <poll.h>
// project includes
#include "he100.h"      /*  exposes the correct serial device location */
#include "fletcher.h"
#include "timer.h"
#include "NamedPipe.h"
#include "SpaceDecl.h"
#include "shakespeare.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
// logging 
#define PROCESS "HE100"
#define LOG_PATH "/home/logs/HE100"
#define MAX_LOG_BUFFER_LEN 255

extern const char *HE_STATUS[34] = {
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

extern const char *CMD_CODE_LIST[32] = {
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

extern const char *if_baudrate[6] = {
    "9600","19200","38400","76800","115200"
};

extern const char *rf_baudrate[5] = {
    "1200","9600","19200","38400"
};

FILE *fdlog; // library log file

/**
 * Function to configure interface
 * @param fdin - the file descriptor representing the serial device
 * REF: http://man7.org/linux/man-pages/man3/termios.3.html
 * REF: http://www.unixguide.net/unix/programming/3.6.2.shtml
 */
int
HE100_configureInterface (int fdin)
{
    struct termios settings;

    // get current settings
    int get_settings = -1;
    if ( ( get_settings = tcgetattr(fdin, &settings) ) < 0 )
    {
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (error, "get config failed: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
        return HE_FAILED_TTY_CONFIG; 
    }
    // attempt to set input and output baud rate to 9600
    if (cfsetispeed(&settings, B9600) < 0 || cfsetospeed(&settings, B9600) < 0)
    {
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (error, "failed set BAUD rate: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
        return HE_FAILED_SET_BAUD;
    }

    // Input flags
    //settings.c_iflag = 0; // disable input processing
    settings.c_iflag &= ~(
          IGNBRK // disable: ignore BREAK condition on input
        | BRKINT // convert break to null byte
        | ICRNL  // no CR to NL translation
        | INLCR  // no NL to CR translation
        | PARMRK // don't mark parity errors or breaks
        | INPCK  // no input parity check
        | ISTRIP // don't strip high bit off
    //    | IXON   // no XON/XOFF software flow control
    );
    settings.c_iflag |= (
          IXON
        | IXOFF
        | IGNPAR // ignore bytes with parity errors
        | ICRNL  // map CR to NL (otherwise CR input on other computer will not terminate input)
    //    | INLCR  // map NL to CR (otherwise CR input on other computer will not terminate input)
    );

    // Output flags
    settings.c_oflag = 0; // disable output processing, raw output

    // Line processing flags
    settings.c_lflag = 0; // disable line processing

    // settings.c_lflag = ICANON; // enable canonical input
    //cfmakeraw(&settings); // raw mode Input is not assembled into lines and special characters are not processed.

    settings.c_lflag = ECHONL;

    // Control flags
    settings.c_cflag &= ~( // disable stuff
          PARENB  // no parity bit
        | CSIZE   // mask the character size bits
        | CSTOPB  // stop bits: 1 stop bit
        | CRTSCTS // disable hardware flow control
        //| ~PARODD; // even parity
    );
    settings.c_cflag |= ( // enable stuff
          B9600    // set BAUD to 9600
        | CS8      // byte size: 8 data bits
        | CREAD   // enable receiver
        //| PARENB  // enable parity bit
        //| PARODD  // odd parity
        //| CLOCAL  // set local mode
    );

    // ONLY for non-canonical read control behaviour
    settings.c_cc[VMIN]  = 1;     // min bytes to return read
    settings.c_cc[VTIME] = 30;    // read timeout in 1/10 seconds

    fcntl(
        fdin,
        F_SETFL,
        FNDELAY // return 0 if no chars available on port (non-blocking)
    ); // immediate reads

    tcflush(fdin, TCIFLUSH); // flush port before persisting changes

    int apply_settings = -1;
    if ( (apply_settings = tcsetattr(fdin, TCSANOW, &settings)) < 0 ) { // apply attributes
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (error, "failed set config: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
        return HE_FAILED_TTY_CONFIG;
    }
    int flush_device = -1;
    if ( (flush_device = tcsetattr(fdin, TCSAFLUSH, &settings)) < 0 ) { // apply attributes
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (error, "failed flush device: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
        return HE_FAILED_FLUSH;
    }
    return 0;
}

int
HE100_openPort(void)
{
    int fdin; // File descriptor for the port

    fdin = open(
        port_address,
          O_RDWR // O_RDWR read and write (CREAD, )
        | O_NOCTTY // port never becomes the controlling terminal
        | O_NDELAY // use non-blocking I/O
        | O_NONBLOCK
        // CLOCAL don't allow control of the port to be changed
    );

    if (fdin == -1) {
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (error, "Unable to open port: %s, %s, %s, %d", port_address, strerror(errno), __func__, __LINE__);
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
        return HE_FAILED_OPEN_PORT;
    }

    if ( !isatty(fdin) ) {
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (error, "Not a serial device: %s, %s, %s, %d", port_address, strerror(errno), __func__, __LINE__);
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
        return HE_NOT_A_TTY;
    }

    // TODO issue NOOP and check device is on

    if ( HE100_configureInterface(fdin) != 0 ) fdin = -1;
      return(fdin);
}

/* Function to close serial device connection at given file descriptor */
int
HE100_closePort(int fdin)
{
    // TODO: Setting the speed to B0 instructs the modem to "hang up".
    if (close(fdin) == -1)
    {
        char error[MAX_LOG_BUFFER_LEN];
        sprintf (error, "Unable to close serial connection: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
        return HE_FAILED_CLOSE_PORT;
    }
    return 0;
}

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

    // TODO // free(bytes); // bytes is always allocated on the stack!

    fflush( NULL ); fsync(fdin); // TODO fdin, is a tty device, ineffective?

    unsigned char response_buffer[MAX_FRAME_LENGTH]; // TODO this will not be used, fault of refactoring decision

    int read_check = 0;
    if (bytes[HE_CMD_BYTE] != CMD_GET_CONFIG) // some commands manually manage reading responses
    { // Issue a read to check for ACK/NOACK
        read_check = HE100_read(fdin, 2, response_buffer);
    }  else read_check = 1;
    // Check if number of bytes written is greater than zero
    // and if read returns a valid message
    if ( read_check >= 0 && w>0 ) {
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
        return HE_INVALID_COMMAND;
    else r = 0; // so far so good

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
            sprintf (output, "ACK>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::NOTICE;
            /* TODO Check the header checksum here, a bit different than payload responses */
            r = 0;
        } else if (response[4] == HE_NOACK) {
            sprintf (output, "NACK>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::ERROR;
            r = HE_FAILED_NACK;
        } else if (response[4] == 0) {
            sprintf (output, "Empty Response>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::ERROR;
            r = HE_EMPTY_RESPONSE;
        } else {
            sprintf (output, "Unknown byte sequence>%s:%s>%d", CMD_CODE_LIST[(int)response[HE_CMD_BYTE]], __func__, __LINE__);
            logPriority = Shakespeare::ERROR;
            r = HE_INVALID_BYTE_SEQUENCE;
        }
        Shakespeare::log_shorthand(LOG_PATH, logPriority, PROCESS, output);
    }
    else
    {
        // something is wrong
        //
        //
        if (h_chk != 0) {
           char error[MAX_LOG_BUFFER_LEN];
           sprintf (
                    error, 
                    "Invalid header checksum. Incoming: [%d,%d] Calculated: [%d,%d] %s, %d", 
                    (uint8_t)response[HE_HEADER_CHECKSUM_BYTE_1],(uint8_t)response[HE_HEADER_CHECKSUM_BYTE_2],
                    (uint8_t)h_chksum.sum1,(uint8_t)h_chksum.sum2, 
                    __func__, __LINE__
            );
            Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
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
            Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
            r=HE_FAILED_CHECKSUM;
        }

        if (payload_length==0) {
            char error[MAX_LOG_BUFFER_LEN];
            sprintf (
                    error, 
                    "Unrecognized byte sequence. Zero length payload should be indicated by zero length byte, or by ACK/NACK %s, %d", 
                    __func__, __LINE__
            );
            Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
            r=-1;
        }
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
    fprintf(fdout,"Dumping %zd bytes: ", size);
    for (j=0;j<size;j++)
    {
        fprintf(fdout,"%02X ",bytes[j]);
        //fprintf(fdout,"%s ",(char*)&bytes[j]);
    }
    fprintf(fdout,"\r\n");
    return;
}

// @param payload - the buffered bytes returned parsed from the radio
// @returns int - # of bytes read
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

    // Read continuously from serial device
    //signal(SIGINT, inthand);

    struct pollfd fds;
    fds.fd = fdin;
    fds.events = POLLIN;
   
    if (fdin==0) return -1;

    while (!timer_complete(&read_timer))
    {
        if ( ( ret_value = poll(&fds, 1, 5) > -1 ) ) // if a byte is ready to be read
        {
            read(fdin, &buffer, 1);
            //if (buffer[0] != 0) printf("i=%u breakcond=%u response=%u buffer=%u \n",i,breakcond,response[i], buffer[0]);
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
                if (i>0) 
                {   // we are at the expected end of our message, time to validate
                    int SVR_result = HE100_validateFrame(response, breakcond);
                    if ( SVR_result == 0 ) 
                    {   // valid frame
                        r = 0; 
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
                        sprintf (error, "Memory allocation problem: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
                        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
                        r=-1;
                    }
                    else
                    {   // something unexpected
                        char error[MAX_LOG_BUFFER_LEN];
                        sprintf (error, "Invalid Data: %d, %d, %s, %d", fdin, SVR_result, __func__, __LINE__);
                        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
                        r=-1;
                       
                        // soft reset the transceiver
                        char log_msg[MAX_LOG_BUFFER_LEN]; // prepare to log result of soft reset
                        if ( HE100_softReset(fdin) == 0 ) {
                            sprintf (log_msg, "Soft Reset written successfully!: %d, %d, %s, %d", fdin, SVR_result, __func__, __LINE__);
                        } 
                        else { 
                            sprintf (log_msg, "Soft Reset FAILED: %d, %d, %s, %d", fdin, SVR_result, __func__, __LINE__);
                        }
                        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, log_msg);
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
            sprintf (error, "Problem with poll(): %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
            Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, error);
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

/**
 * TODO COMPLETE AND INTEGRATE
 * Function to decode validated and extracted data from response for 
 * nice outputting or logging
 *
 * @param response - the response data to interpret
 * @param length - the length of the data in bytes
 */
int
HE100_interpretResponse (unsigned char *response, size_t length)
{
    printf("Response length: %d\n", (int)length);

    // if response == transmission, He100 device is off!
    if ((int)response[2]==16) {
        fprintf(stderr,"HE100_read: He100 is off! Line:%d", __LINE__);
        return 0;
    }

    const char* value;
    size_t i=0;
    for (i=0; i<length; i++) // only runs once!
    {
        printf("0x%02X : %d ",response[i], response[i]);
        // instead of this, build array from struct and lookup based on position and value
        // log failed transmissions with shakespeare
        switch ((int)response[i])
        {
            case 72  :
                value = (i==0) ? "Sync H" : "data"; break;
            case 101 :
                value = (i==1) ? "Sync e" : "data"; break;
            case 32  :
                value = (i==2) ? "Response" : "data"; break;
            case 3   :
                value = (i==3) ? "Transmission" : "data"; break;
            case 10  :
                value = (i==4||i==5) ? "Acknowledge" : "data"; break;
            default  :
                value = (i==length||i==length-1) ?"chksum" :"data"; break;
        }
        printf(": %s,\n",value);
    }
    return 1;
}

int 
HE100_dispatchTransmission(int fdin, unsigned char *payload, size_t payload_length, unsigned char *command)
{
    // initialize transmission array to store prepared byte sequence
    unsigned char transmission[MAX_FRAME_LENGTH] = {0};
    memset (transmission,'\0',MAX_FRAME_LENGTH);    
    
    // if preparation successful, write the bytes to the radio
    if (HE100_prepareTransmission(payload,transmission,payload_length,command) == 0)
         return HE100_write(fdin,transmission,payload_length+WRAPPER_LENGTH);
    else return 1;
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
   if (power_level > MAX_POWER_LEVEL || power_level < MIN_POWER_LEVEL) return 1;
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

// TODO these settings are not clearly defined in documentation and need to be confirmed
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

    settings.function_config = buffer[CFG_RX_CRC_BYTE]; 
    //settings.dio_pin13 = buffer[CFG_RX_CRC_BYTE]; 
    //settings.rx_crc = buffer[CFG_DIO_PIN13_BYTE]; 
    //settings.led_blink_type = buffer[CFG_LED_BYTE]; 

    // TODO settings.function_config 
    settings.rxtx_test_cw = buffer[CFG_RXTX_TEST_CW_BYTE]; 
    settings.ext_conf_setting = buffer[CFG_EXT_BYTE]; 

    //memcpy (&settings,buffer+WRAPPER_LENGTH,CFG_PAYLOAD_LENGTH); // copies char buf into struct
    return settings;
}

void 
HE100_printSettings( struct he100_settings settings ) {
    printf("Interface Baud Rate: %s [%d]\n\r", if_baudrate[settings.interface_baud_rate],settings.interface_baud_rate);
    printf("TX Power Amplification Level: %d [%d] \r\n", settings.tx_power_amp_level*100/255,settings.tx_power_amp_level);
    printf("RX Baud Rate: %s [%d] \r\n", rf_baudrate[settings.rx_rf_baud_rate],settings.rx_rf_baud_rate);
    printf("TX Baud Rate: %s [%d] \r\n", rf_baudrate[settings.tx_rf_baud_rate],settings.tx_rf_baud_rate);
    printf("RX Frequency: %d \r\n", settings.rx_freq);
    printf("TX Frequency: %d \r\n", settings.tx_freq);

    char dio_pin13_value[64] = {0};
    switch (settings.dio_pin13)
    {
        case CFG_RX_CRC_ON: // 0x43
        //case CFG_LED_RX: // 0x43
        //case CFG_DIO_PIN13_OFF: // 0x43
           sprintf(dio_pin13_value, "CRC ON, DIO_PIN13 OFF, CFG_LED_RX");
           break;
        case CFG_RX_CRC_OFF: // 0x03
           sprintf(dio_pin13_value, "CRC OFF");
           break; 
        case CFG_DIO_PIN13_TXRXS: // 0x47
           sprintf(dio_pin13_value, "DIO_PIN13 TXRXS");
           break; 
        case CFG_DIO_PIN13_2p5HZ: // 0x4b
           sprintf(dio_pin13_value, "DIO_PIN13 2p5HZ");
           break; 
        case CFG_DIO_PIN13_RXTOG: // 0x4f
           sprintf(dio_pin13_value, "DIO_PIN13 RXTOG");
           break; 
        case CFG_LED_PS: // 0x41
           sprintf(dio_pin13_value, "CFG_LED Pulse");
           break; 
        case CFG_LED_TX: // 0x42
           sprintf(dio_pin13_value, "CFG_LED on Transmit");
           break; 
        default : 
           sprintf(dio_pin13_value, "Invalid DIO_PIN 13 Value: %02X");
           break;
    }
    printf("DIO_PIN13 Behavior: %s [%02X] \r\n", dio_pin13_value, settings.dio_pin13);

    printf("Source Callsign: %s \r\n", settings.source_callsign);
    printf("Destination Callsign: %s \r\n", settings.destination_callsign);
    printf("TX Preamble: %d \r\n", settings.tx_preamble);
    printf("TX Postamble: %d \r\n", settings.tx_postamble);
   
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
    printf("EXT: %s [%02X] \r\n", ext_conf_value, settings.ext_conf_setting);
}

int 
HE100_prepareConfig (unsigned char * prepared_bytes, struct he100_settings settings) {
/*    
    // TODO should this be changed with the serial connection as well?
    memcpy(prepared_bytes[CFG_IF_BAUD_BYTE],settings.interface_baud_rate,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_PA_BYTE],settings.tx_power_amp_level,sizeof(uint8_t);
    memcpy(prepared_bytes[CFG_RF_RX_BAUD_BYTE],settings.rx_rf_baud_rate,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_RF_TX_BAUD_BYTE],settings.tx_rf_baud_rate,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_RX_MOD_BYTE],settings.rx_modulation,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_TX_MOD_BYTE],settings.tx_modulation,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_RX_FREQ_BYTE1],settings.rx_freq,sizeof(uint16_t));
    memcpy(prepared_bytes[CFG_TX_FREQ_BYTE1],settings.tx_freq,sizeof(uint16_t));
    
    memcpy(prepared_bytes[CFG_SRC_CALL_BYTE],settings.source_callsign,CFG_CALLSIGN_LEN);
    memcpy(prepared_bytes[CFG_DST_CALL_BYTE],settings.destination_callsign,CFG_CALLSIGN_LEN);

    memcpy(prepared_bytes[CFG_RX_CRC_BYTE],settings.rx_crc,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_LED_BYTE],settings.led_blink_type,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_RXTX_TEST_CW_BYTE],settings.rxtx_test_cw,sizeof(uint8_t));
    memcpy(prepared_bytes[CFG_DIO_PIN13_BYTE],settings.dio_pin13,sizeof(uint8_t));

    switch (settings.ext_conf_setting)
    {
        case 0 : / all EXT functions off 
           prepared_bytes[CFG_EXT_BYTE] = 0;
           break; 
        case CFG_EXT_PING_ON: // 
           prepared_bytes[CFG_EXT_BYTE]=CFG_EXT_PING_ON; break; 
        case CFG_EXT_CODEUPLOAD_ON: // 
           prepared_bytes[CFG_EXT_BYTE]=CFG_EXT_CODEUPLOAD_ON;
           break; 
        case CFG_EXT_RESET_ON: //
           prepared_bytes[CFG_EXT_BYTE]=CFG_EXT_RESET_ON;
           break; 
        default : 
           prepared_bytes[CFG_EXT_BYTE]=CFG_EXT_DEF;
           return HE_INVALID_EXT;
           break;
    }
*/
    memcpy (prepared_bytes,&settings,CFG_PAYLOAD_LENGTH); // copies char buf into struct
    return 0;
}

// TODO redundant to call two structs. What do we want to DO with these settings? Perhaps write to file.
int 
HE100_getConfig (int fdin)
{
    int result = 1;
    struct he100_settings settings;
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
         settings = HE100_collectConfig(config_bytes); 
         HE100_printSettings( settings );
         result = HE100_validateConfig(settings); 
       } else result = HE_FAILED_READ;
    }
    return result;
}

/**  
 *  This function validates a he100_settings struct
 *  and converts to char array for safe deployment
 **/ 
int 
//HE100_validateConfig (struct he100_settings he100_new_settings,unsigned char* set_config_payload)
HE100_validateConfig (struct he100_settings he100_new_settings)
{
    char validation_log_entry[MAX_LOG_BUFFER_LEN];

    // validate new interface baud rate setting
    if (
            he100_new_settings.interface_baud_rate <= MAX_IF_BAUD_RATE 
        //&&  he100_new_settings.interface_baud_rate >= MIN_IF_BAUD_RATE // always true
        // TODO WHY // &&  he100_new_settings.interface_baud_rate != CFG_DEF_IF_BAUD 
    ) 
    {
    }
    else {
        sprintf(validation_log_entry,"%s %s ^%s@%d",
                HE_STATUS[HE_INVALID_IF_BAUD_RATE],if_baudrate[he100_new_settings.interface_baud_rate],
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_IF_BAUD_RATE;
    } 
    

    // validate new power amplification level
    if (
               he100_new_settings.tx_power_amp_level <= MAX_PA_LEVEL
            && he100_new_settings.tx_power_amp_level >= MIN_PA_LEVEL // always true
       ) 
    {
    } 
    else { 
        sprintf(validation_log_entry,"%s BAUD:%d ^%s@%d",
                HE_STATUS[HE_INVALID_POWER_AMP_LEVEL],he100_new_settings.tx_power_amp_level,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_POWER_AMP_LEVEL;
    };

    // validate new rf baud rates
    if (
            he100_new_settings.rx_rf_baud_rate <= MAX_RF_BAUD_RATE
        //&&  he100_new_settings.rx_rf_baud_rate >= MIN_RF_BAUD_RATE // always true
        &&  he100_new_settings.tx_rf_baud_rate <= MAX_RF_BAUD_RATE
        //&&  he100_new_settings.tx_rf_baud_rate >= MIN_RF_BAUD_RATE // always true
    ) 
    {
    }
    else {
        sprintf(validation_log_entry,"%s MAX:%d MIN:%d INDEX:%d BAUD:%s ^%s@%d",
                HE_STATUS[HE_INVALID_RF_BAUD_RATE],
                MAX_RF_BAUD_RATE, MIN_RF_BAUD_RATE,
                he100_new_settings.tx_power_amp_level,rf_baudrate[he100_new_settings.tx_power_amp_level],
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_RF_BAUD_RATE;
    }
   
    // validate modulation (USE DEFAULTS)
    if (
            he100_new_settings.rx_modulation == CFG_RX_DEF_MOD
    )
    {
    }
    else {
        sprintf(validation_log_entry,"%s [rx] expected[%d] actual[%d] ^%s@%d",
                HE_STATUS[HE_INVALID_RX_MOD],
                CFG_RX_DEF_MOD,
                he100_new_settings.rx_modulation,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_RX_MOD; // TODO what about TX_MOD?
    }

    if (
         he100_new_settings.tx_modulation == CFG_TX_DEF_MOD        
    )
    {
    }
    else {
        sprintf(validation_log_entry,"%s expected[%d] actual[%d] ^%s@%d",
                HE_STATUS[HE_INVALID_TX_MOD],
                CFG_TX_DEF_MOD,
                he100_new_settings.tx_modulation,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_TX_MOD; // TODO what about TX_MOD?
    }

    // validate new RX setting
    if ( 
        (he100_new_settings.rx_freq >= MIN_UPPER_FREQ && he100_new_settings.rx_freq <= MAX_UPPER_FREQ)
     || (he100_new_settings.rx_freq >= MIN_LOWER_FREQ && he100_new_settings.rx_freq <= MAX_LOWER_FREQ)
    )
    {
    }
    else { 
        sprintf(validation_log_entry,"%s rx:%d ^%s@%d",
                HE_STATUS[HE_INVALID_RX_FREQ],he100_new_settings.rx_freq,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_RX_FREQ;
    }

    // validate new TX setting
    if ( 
        (he100_new_settings.tx_freq >= MIN_UPPER_FREQ && he100_new_settings.tx_freq <= MAX_UPPER_FREQ)
     || (he100_new_settings.tx_freq >= MIN_LOWER_FREQ && he100_new_settings.tx_freq <= MAX_LOWER_FREQ)
    )
    {
    }
    else {
        sprintf(validation_log_entry,"%s tx:%d ^%s@%d",
                HE_STATUS[HE_INVALID_TX_FREQ],he100_new_settings.tx_freq,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_TX_FREQ;
    }

    // validate callsigns (USE DEFAULTS) 
    if ( //1
        // length should be 6, valid CALLSIGN
        ( memcmp(he100_new_settings.source_callsign, CFG_SRC_CALL_DEF, 1) == 0 )
     && ( memcmp(he100_new_settings.destination_callsign, CFG_DST_CALL_DEF, 1) == 0 )
    )
    { }
    else {
        sprintf(validation_log_entry,"%s source:%s destination:%s ^%s@%d",
                HE_STATUS[HE_INVALID_CALLSIGN],he100_new_settings.source_callsign,he100_new_settings.destination_callsign,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_CALLSIGN;
    }
  
/***   DIO PIN 13, RX CRC, LED, this doesn't make much sense yet:  *******************/
/* the following settings are in conflict because they are set by the same byte */
/* CFG_RX_CRC_BYTE == CFG_LED_BYTE == CFG_DIO_PIN13_BYTE == 30 */
/* 0x43 could conceivably simultaneously enable CFG_RX_CRC_ON, CFG_LED_RX, and CFG_DIO_PIN13_OFF */
    if (
            he100_new_settings.function_config == CFG_RX_CRC_ON
         || he100_new_settings.function_config == CFG_RX_CRC_OFF
         || he100_new_settings.function_config == CFG_LED_PS
         || he100_new_settings.function_config == CFG_LED_RX
         || he100_new_settings.function_config == CFG_DIO_PIN13_OFF
         || he100_new_settings.function_config == CFG_DIO_PIN13_TXRXS
         || he100_new_settings.function_config == CFG_DIO_PIN13_2p5HZ
         || he100_new_settings.function_config == CFG_DIO_PIN13_RXTOG
       ) 
    {
    }
    else {
        sprintf(validation_log_entry,"%s setting:%02X ^%s@%d",
                HE_STATUS[HE_INVALID_DIO_PIN13],
                he100_new_settings.dio_pin13,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_DIO_PIN13;
    }


/*************************************************************************************/
    // validate TX Test CW (USE DEFAULTS) 
    if ( he100_new_settings.rxtx_test_cw == CFG_RXTX_TEST_CW_DEF )
    {
    }
    else { 
        sprintf(validation_log_entry,"%s setting:%02X ^%s@%d",
                HE_STATUS[HE_INVALID_RXTX_TEST],
                he100_new_settings.rxtx_test_cw,
                __func__,__LINE__
        );
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::ERROR, PROCESS, validation_log_entry);
        return HE_INVALID_RXTX_TEST;
    }

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
    int validate_result = HE100_validateConfig(he100_new_settings);
    unsigned char set_config_command[2] = {CMD_TRANSMIT, CMD_SET_CONFIG};

    if (validate_result == 0) // we have valid array
        return HE100_dispatchTransmission(fdin, set_config_payload, CFG_PAYLOAD_LENGTH, set_config_command);
    else return HE_INVALID_CONFIG;
}

int 
HE100_writeFlash (int fdin, unsigned char *flash_md5sum)
{
    HE100_dumpHex (stdout,flash_md5sum,24); // TODO handle this properly, see docs

    unsigned char write_flash_payload[CFG_FLASH_LENGTH] = {0};
    unsigned char write_flash_command[2] = {CMD_TRANSMIT, CMD_WRITE_FLASH};
    
    return HE100_dispatchTransmission(fdin, write_flash_payload, CFG_FLASH_LENGTH, write_flash_command); 
}
