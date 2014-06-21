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
#include "SC_he100.h"   /*  Helium 100 header file */
#include "time.h"
#include <poll.h>
// project includes
#include "he100.h"      /*  exposes the correct serial device location */
#include "fletcher.h"
//#include "./Net2Com.h"
#include "timer.h"
#include "NamedPipe.h"
#include "SpaceDecl.h"
#include "shakespeare.h"

#define LOG_PATH "/home/logs/HE100"
#define DATA_PIPE_PATH "/var/log/he100/data.log"
static NamedPipe datapipe("/var/log/he100/data.log");

#define PROCESS "HE100"

// baudrate settings are defined in <asm/termbits.h> from <termios.h>
#define MAX_FRAME_LENGTH 255
#define MAX_TESTED_PAYLOAD 190
#define BAUDRATE B9600
#define TTYDEVICE "/dev/ttyS2"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define PARITYBIT ~PARENB // no parity bit
#define BYTESIZE CS8 // 8 data bits
#define STOPBITS ~CSTOPB // 1 stop bit
#define HWFLWCTL ~CRTSCTS // disable hardware flow control

#define CFG_OFF_LOGIC LOW   0x00

#define NOPAY_COMMAND_LENGTH 8
#define WRAPPER_LENGTH       10

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

// LED config
#define CFG_LED_BYTE 38  // 38th byte in byte array
#define CFG_LED_PS  0x41 // 2.5 second pulse
#define CFG_LED_TX  0x42 // flash on transmit
#define CFG_LED_RX  0x43 // flash on receive
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
// EX {48 65 10 05 ... } request config
// EX {48 65 20 05 ... } actual config
#define CMD_SET_CONFIG          0x06 // followed by config bytes
// EX {48 65 10 06 ... checksum }
// EX {48 65 20 06 ... } -> ACK
#define CMD_TELEMETRY           0x07 // query a telemetry frame
// EX {48 65 10 06 ... } query a telemetry frame
// EX {48 65 20 06 ... } receive a telemetry frame
#define CMD_WRITE_FLASH         0x08 // write flash 16 byte MDF
#define CMD_RF_CONFIGURE        0x09 // Low Level RF Configuration
#define CMD_BEACON_DATA         0x10 // Set Beacon Message
#define CMD_BEACON_CONFIG       0x11 // Set Beacon configuration
#define CMD_READ_FIRMWARE_V     0x12 // read radio firmware revision
// EX {48 65 10 12 ... } request revision number
// EX {48 65 20 12 REV } float 4 byte revision number
#define CMD_DIO_KEY_WRITE       0x13
#define CMD_FIRMWARE_UPDATE     0x14
#define CMD_FIRMWARE_PACKET     0x15
#define CMD_FAST_SET_PA         0x20

FILE *fdlog; // library log file
FILE *fdata; // pipe to send valid payloads for external use
int f_fdata_int; // file descriptor for pipe


static int pipe_initialized = FALSE;

/**
 * Function to configure interface
 * @param fdin - the file descriptor representing the serial device
 * REF: http://man7.org/linux/man-pages/man3/termios.3.html
 * REF: http://www.unixguide.net/unix/programming/3.6.2.shtml
 */

void pipe_init(){
   if(!pipe_initialized){
      if (!datapipe.Exist()) datapipe.CreatePipe();
      pipe_initialized = TRUE;
   }
}

void
HE100_configureInterface (int fdin)
{
    struct termios settings;

    // get current settings
    int get_settings = -1;
    if ( ( get_settings = tcgetattr(fdin, &settings) ) < 0 )
    {
        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
        char error[255];
        sprintf (error, "config failed: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
        fclose(fdlog);
        exit(EXIT_FAILURE);
    }

    // attempt to set input and output baud rate to 9600
    if (cfsetispeed(&settings, B9600) < 0 || cfsetospeed(&settings, B9600) < 0)
    {
        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
        char error[255];
        sprintf (error, "failed set BAUD rate: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
        fclose(fdlog);
        exit(EXIT_FAILURE);
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
        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
        char error[255];
        sprintf (error, "failed set config: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
        fclose(fdlog);
        exit(EXIT_FAILURE);
    }

    int flush_device = -1;
    if ( (flush_device = tcsetattr(fdin, TCSAFLUSH, &settings)) < 0 ) { // apply attributes
        fprintf(stderr, "\r\nHE100_configureInterface: failed to flush device: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);
    }
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
        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
        char error[255];
        sprintf (error, "Unable to open port: %s, %s, %s, %d", port_address, strerror(errno), __func__, __LINE__);
        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
        fclose(fdlog);        
        return HE_FAILED_OPEN_PORT;
    }

    if ( !isatty(fdin) ) {
        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
        char error[255];
        sprintf (error, "Not a serial device: %s, %s, %s, %d", port_address, strerror(errno), __func__, __LINE__);
        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
        fclose(fdlog); 

        return HE_NOT_A_TTY;
    }

    // TODO issue NOOP and check device is on

    HE100_configureInterface(fdin);

    return(fdin);
}

/* Function to close serial device connection at given file descriptor */
int
HE100_closePort(int fdin)
{
    // TODO: Setting the speed to B0 instructs the modem to "hang up".
    if (close(fdin) == -1)
    {
        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
        char error[255];
        sprintf (error, "Unable to close serial connection: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
        fclose(fdlog);
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
    // Issue a read to check for ACK/NOACK

    unsigned char response_buffer[MAX_FRAME_LENGTH]; // TODO this will not be used, fault of refactoring decision
    int read_check = HE100_read(fdin, 2, response_buffer);

    // Check if number of bytes written is greater than zero
    // and if read returns a valid message
    if ( read_check == 0 && w>0 ) {
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
    size_t payload_length = length - WRAPPER_LENGTH; // response minus header minus 4 checksum bytes and 2 sync bytes and 2 length bytes

    // validate length
    if ( response[HE_LENGTH_BYTE] != payload_length ) return CS1_WRONG_LENGTH;    

/*  TODO memory should not need to be allocated to apply checksum   */
/*  simply pass the range of bytes to the checksum function!!       */
    // assign memory to hold transmission minus sync bytes 
    unsigned char *data = (unsigned char *) malloc(length);
    if (data==NULL) return CS1_NULL_MALLOC;

    // assign memory to hold message (transmission minus all meta data)
    unsigned char *msg = (unsigned char *) malloc(data_length);
    if (msg==NULL) return CS1_NULL_MALLOC;

    // copy the header into the new response array minus sync bytes
    size_t i; size_t j=0;
    for (i=2;i<8;i++) {
        data[j] = response[i];
        j++;
    }

    // generate and compare header checksum
    fletcher_checksum h_chksum = fletcher_checksum16(data,4);
    uint8_t h_s1_chk = memcmp(&response[HE_HEADER_CHECKSUM_BYTE_1], &h_chksum.sum1, 1);
    uint8_t h_s2_chk = memcmp(&response[HE_HEADER_CHECKSUM_BYTE_2], &h_chksum.sum2, 1);
    int h_chk = h_s1_chk + h_s2_chk; // should be zero given valid chk

    // pick up where j left off
    for (i=8;i<data_length;i++) /* read up to, not including payload chksum */
    {
        data[j] = response[i];
        j++;
    }

    // generate and compare payload checksum
    fletcher_checksum p_chksum = fletcher_checksum16(data,data_length-2); // chksum everything except 'He' and payload checksum bytes
    uint8_t p_s1_chk = memcmp(&response[pb1], &p_chksum.sum1, 1);
    uint8_t p_s2_chk = memcmp(&response[pb2], &p_chksum.sum2, 1);
    int p_chk = p_s1_chk + p_s2_chk; // should be zero given valid chk
/*  END TODO    */

    if (response[4] == response[5] ) /* ACK or NOACK or EMPTY length */
    {
        if (response[4] == 10) {
            fprintf(stdout,"\r\n  HE100: Acknowledge");
            /* TODO Check the header checksum here, a bit different than payload responses */
            r = 0;
        } else if (response[4] == 255) {
            fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
            char error[255];
            sprintf (error, "NACK: %s, %d", __func__, __LINE__);
            if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
            fclose(fdlog); 
            r = 1;
        } else {
            fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
            char error[255];
            sprintf (error, "Unknown byte sequence: %s, %d", __func__, __LINE__);
            if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
            fclose(fdlog);
            r = 1;
        }
    }
    else
    {
        if (h_chk != 0) {
            fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
            char error[255];
            sprintf (
                    error, 
                    "Invalid header checksum. Incoming: [%d,%d] Calculated: [%d,%d] %s, %d", 
                    (uint8_t)response[HE_HEADER_CHECKSUM_BYTE_1],(uint8_t)response[HE_HEADER_CHECKSUM_BYTE_2],
                    (uint8_t)h_chksum.sum1,(uint8_t)h_chksum.sum2, 
                    __func__, __LINE__
            );
            if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
            fclose(fdlog);
            // DISABLED FOR TESTING, TODO, FIX! r=1;
        }

        if (p_chk != 0) {
            fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
            char error[255];
            sprintf (
                    error, 
                    "Invalid payload checksum. Incoming: [%d,%d] Calculated: [%d,%d] %s, %d", 
                    (uint8_t)response[pb1],(uint8_t)response[pb2],(uint8_t)p_chksum.sum1,(uint8_t)p_chksum.sum2,
                    __func__, __LINE__
            );
            if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
            // DISABLED FOR TESTING, TODO, FIX! // r=1;
        }
    }

    free(data);
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

/** Provide signal handling for SC_read **/
//volatile sig_atomic_t stop;
//void inthand (int signum) { stop = 1; }

/**
 * Function to read bytes in single-file from the serial device and
 * append them to and return a response array
 *
 * @param fdin - the file descriptor representing the serial device
 * @param response - a buffer you pass with 255 bytes of memory in which to place
 *  the response data
 * @return - the length of the payload read
 */
int
HE100_read (int fdin, time_t timeout, unsigned char * payload)
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
    timer_start(&read_timer,timeout,0);

    // Variables for select
    int ret_value;

    // Read continuously from serial device
    //signal(SIGINT, inthand);

    struct pollfd fds;
    fds.fd = fdin;
    fds.events = POLLIN;
   
    if (fdin==0) return 1;

    while (!timer_complete(&read_timer))
    {
        if ( ( ret_value = poll(&fds, 1, 5) > -1 ) ) // if a byte is ready to be read
        {
            read(fdin, &buffer, 1);
            if (buffer[0] != 0) printf("i=%u breakcond=%u response=%u buffer=%u \n",i,breakcond,response[i], buffer[0]);

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
                if (i>0) // we are at the expected end of our message, time to validate
                {
                    int SVR_result = HE100_validateFrame(response, breakcond);
                    if ( SVR_result == 0 ) 
                    {   // valid frame
                        r = 0; 
                        size_t payload_length = breakcond - WRAPPER_LENGTH;
                        memcpy (payload, response+HE_FIRST_PAYLOAD_BYTE, payload_length);
                        r=payload_length;
                    }
                    else if (SVR_result == CS1_NULL_MALLOC) 
                    {   // memory allocation problem in validateFrame()
                        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
                        char error[255];
                        sprintf (error, "Memory allocation problem: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
                        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
                        r=-1;//r=CS1_NULL_MALLOC;
                    }
                    else
                    {   // something unaccounted for
                        fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
                        char error[255];
                        sprintf (error, "Invalid Data: %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
                        if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);

                        //r=CS1_INVALID_BYTE_SEQUENCE;
                        r=-1;

                        // soft reset the transceiver
                        if ( HE100_softReset(fdin) > 0 ) printf("\r\n Soft Reset written successfully!");
                        else printf("\r\n Problems with soft reset");
                    }
                    break; // TODO why is this break needed?
                }

                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
            }
            buffer[0] = '\0'; // wipe buffer each time
        }
        else if (ret_value == -1) {
            // bad or no read
            fdlog = Shakespeare::open_log(LOG_PATH,PROCESS);
            char error[255];
            sprintf (error, "Problem with poll(): %d, %s, %s, %d", fdin, strerror(errno), __func__, __LINE__);
            if (fdlog != NULL) Shakespeare::log(fdlog, Shakespeare::ERROR, PROCESS, error);
            r = -1;
        }
    }
    return r;
}

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of incoming data
 */
// TODO //int* HE100_prepareTransmission(unsigned char *payload, unsigned char *prepared_transmission, size_t length, unsigned char *command)
unsigned char*
HE100_prepareTransmission(unsigned char *payload, size_t length, unsigned char *command)
{
    // TODO how to check if length is not accurate?
    //if (command[length-1]==0xFF) {
    //    printf("Wrong length");
    //    exit(EXIT_FAILURE);
    //}

    size_t transmission_length;
    size_t payloadbytes_length;
    size_t i; // payload index
    int payload_chksum_bool;

    // set the array bounds based on command
    if (command[1] == 0x01 || command[1] == 0x02 || command[1] == 0x12 || command[1] == 0x05) /* empty payload */ {
        transmission_length = NOPAY_COMMAND_LENGTH;
        payloadbytes_length = 6;
        payload_chksum_bool = 0;
    } else {
        transmission_length = length+WRAPPER_LENGTH;
        payloadbytes_length = length+8;
        payload_chksum_bool = 1;
    }

    // transmission will contain the entire byte sequence to be sent
    unsigned char *transmission = (unsigned char *) malloc(transmission_length); // TODO free me!
    if (transmission==NULL) exit(EXIT_FAILURE);

    // payloadbytes will contain the byte sequence to be sent minus the sync bytes which are not checksummed
    unsigned char *payloadbytes = (unsigned char *) malloc(payloadbytes_length);
    if (payloadbytes==NULL) exit(EXIT_FAILURE);
    
    // TODO consider that leaving the first two bytes of the transmission empty would not affect
    // the checksum values, and would clarify the code

    // attach sync bytes to final transmission byte array
    transmission[0] = SYNC1; //0x48;
    transmission[1] = SYNC2; //0x65;

    // attach command bytes to intermediary payload byte array
    payloadbytes[0] = (unsigned char) command[0];
    payloadbytes[1] = (unsigned char) command[1];

    // attach length bytes
    //payloadbytes[2] = 0x00;
    payloadbytes[2] = length >> 8;
    payloadbytes[3] = (unsigned char) length & 0xff;

    // generate and attach header checksum
    fletcher_checksum header_checksum = fletcher_checksum16(payloadbytes,4);
    payloadbytes[4] = (unsigned char) header_checksum.sum1 & 0xff;
    payloadbytes[5] = (unsigned char) header_checksum.sum2 & 0xff;

    if( payload_chksum_bool==1 ) // real payload
    {
        // attach data to payload
        for (i=0;i<length;i++) { /* or use memcpy with offset */
            payloadbytes[6+i] = payload[i];
        }
        // generate and attach payload checksum
        fletcher_checksum payload_checksum = fletcher_checksum16(payloadbytes,length+6); // chksum everything except first two bytes 'He'
        payloadbytes[6+length] = payload_checksum.sum1;
        payloadbytes[6+length+1] = payload_checksum.sum2;
    }

    // attach payload and return final transmission
    size_t j=0;
    for ( i=2; i<transmission_length; i++ ) {
        transmission[i] = payloadbytes[j];
        j++;
    }// or use memcpy with offset

    free(payloadbytes);
    return (unsigned char*) transmission;
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
 *
 * Most responses are 8 bytes ack or no ack
 * noop_ack         486520010a0a35a1
 * noop_noack       48652001ffff1f80
 * setconfig_ack    486520060a0a3ab0
 * tx_ac            486520030a0a37a7
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

/**
 * Function to return NOOP byte sequence
 * no arguments
 */
int
HE100_NOOP (int fdin)
{
   unsigned char noop_payload[1] = {0};
   unsigned char noop_command[2] = {CMD_TRANSMIT, CMD_NOOP};
   return HE100_write(
        fdin,
        HE100_prepareTransmission(
                noop_payload,
                0,
                noop_command
        ), 10
    );
}

/**
 * Function returning byte sequence to set the beacon message
 * unsigned char *beacon_message_payload message to transmit
 */
int
HE100_transmitData (int fdin, unsigned char *transmit_data_payload, size_t transmit_data_len)
{
    unsigned char transmit_data_command[2] = {CMD_TRANSMIT, CMD_TRANSMIT_DATA};
    return HE100_write(
        fdin,
        HE100_prepareTransmission(
                transmit_data_payload,
                transmit_data_len,
                transmit_data_command
        ), transmit_data_len+10
    );
}

/**
 * Function returning byte sequence to enable beacon on given interval
 * int beacon_interval interval in seconds
 */
int
HE100_setBeaconInterval (int fdin, int beacon_interval)
{
   unsigned char beacon_interval_payload[1];
   beacon_interval_payload[0] = beacon_interval & 0xff;

   if (beacon_interval > 255 ) {
        return -1;
   }

   unsigned char beacon_interval_command[2] = {CMD_TRANSMIT, CMD_BEACON_CONFIG};
   return HE100_write(
        fdin,
        HE100_prepareTransmission(
                beacon_interval_payload,
                1,
                beacon_interval_command
        ), 1+10
    );
}

/**
 * Function returning byte sequence to set the beacon message
 * unsigned char *beacon_message_payload message to transmit
 */
int
HE100_setBeaconMessage (int fdin, unsigned char *set_beacon_message_payload, size_t beacon_message_len)
{
   unsigned char set_beacon_message_command[2] = {CMD_TRANSMIT, CMD_BEACON_DATA};
   return HE100_write(
        fdin,
        HE100_prepareTransmission(
                set_beacon_message_payload,
                beacon_message_len,
                set_beacon_message_command
        ), beacon_message_len+10
    );
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
   if (power_level > 255) {
      return -1;
   }

   unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
   return HE100_write(
        fdin,
        HE100_prepareTransmission(
                fast_set_pa_payload,
                1,
                fast_set_pa_command
        ), 1+10
    );
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
   return HE100_write(
        fdin,
        HE100_prepareTransmission(
                soft_reset_payload,
                0,
                soft_reset_command
        ), 10
    );
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
   return HE100_write(
        fdin,
        HE100_prepareTransmission(
                read_firmware_revision_payload,
                0,
                read_firmware_revision_command
        ), 10
    );
}
