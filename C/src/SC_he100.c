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

#define LOG_FILE_PATH "/var/log/he100/he100.log"
#define DATA_PIPE_PATH "/var/log/he100/data.log"
static NamedPipe datapipe("/var/log/he100/data.log");

#define EMPTY_PAYLOAD_WRITE_LENGTH 8

// baudrate settings are defined in <asm/termbits.h> from <termios.h>
#define MAX_FRAME_LENGTH 255
#define TTYDEVICE "/dev/ttyS2"
#define CFG_DEFAULT_BAUDRATE B9600
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

// Config options
#define CFG_FRAME_LENGTH    44
#define CFG_PAYLOAD_LENGTH  34
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
#define MAX_PA_LEVEL        0xFF
#define MIN_PA_LEVEL        0x00
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
#define CFG_RX_DEF_MOD      0x00 // GFSK
#define CFG_TX_DEF_MOD      0x00 // GFSK
// RX TX FREQ config
#define MAX_UPPER_FREQ      450000 
#define MIN_UPPER_FREQ      400000
#define MAX_LOWER_FREQ      150000
#define MIN_LOWER_FREQ      120000
#define CFG_RX_FREQ_BYTE1   6 // 7th byte
#define CFG_RX_FREQ_BYTE2   7 // 8th byte
#define CFG_RX_FREQ_DEFAULT 144200
#define CFG_TX_FREQ_BYTE1   10 // 11th byte
#define CFG_TX_FREQ_BYTE2   11 // 12th byte
#define CFG_TX_FREQ_DEFAULT 431000 
// CALLSIGN config
#define CFG_SRC_CALL_BYTE   14 // 15th byte  
#define CFG_DST_CALL_BYTE   20 // 21st byte
#define CFG_SRC_CALL_DEF    "VA3ORB"
#define CFG_DST_CALL_DEF    "VE2CUA"
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
#define CFG_RX_POSTAM_BYTE  28 // 29th byte
#define CFG_RX_POSTAM_DEF   0
#define CFG_RX_POSTAM_MIN   0
#define CFG_RX_POSTAM_MAX   10

// RX CRC config
#define CFG_RX_CRC_BYTE     30 // 31st byte
#define CFG_RX_CRC_ON       0x43
#define CFG_RX_CRC_OFF      0x03

// RX CRC config// DIO - Pin 13 config
#define CFG_DIO_PIN13_BYTE  30 // 31st byte
#define CFG_DIO_PIN13_OFF   0x43
#define CFG_DIO_PIN13_TXRXS 0x47 
#define CFG_DIO_PIN13_2p5HZ 0x4b
#define CFG_DIO_PIN13_RXTOG 0x4f

// TX Test CW config
#define CFG_RXTX_TEST_CW_BYTE 32 // 33rd byte
#define CFG_RXTX_TEST_CW_DEF  0x00 
#define CFG_RXTX_TEST_CW_OFF  0x00 
#define CFG_TX_TEST_CW_ON     0x02 
#define CFG_RX_TEST_CW_ON     0x04

// EXT Functions config
#define CFG_EXT_BYTE          33 // 34th byte
#define CFG_EXT_DEF           0x00  
#define CFG_EXT_OFF           0x00
#define CFG_EXT_PING_ON       0x10
#define CFG_EXT_CODEUPLOAD_ON 0x20
#define CFG_EXT_RESET_ON      0x40

// LED config
#define CFG_LED_BYTE 38  // 38th byte in byte array
#define CFG_LED_PS  0x41 // 2.5 second pulse
#define CFG_LED_TX  0x42 // flash on transmit
#define CFG_LED_RX  0x43 // flash on receive


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
        fprintf(
            stderr,
            "\r\nHE100_configureInterface: failed to get config: %d, %s\n",
            fdin,
            strerror(errno)
        );
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nHE100_configureInterface: successfully acquired old settings");

    // attempt to set input and output baud rate to 9600
    if (cfsetispeed(&settings, B9600) < 0 || cfsetospeed(&settings, B9600) < 0)
    {
        fprintf(stderr, "\r\nHE100_configureInterface: failed set BAUD rate: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nHE100_configureInterface: successfully set new baud rate");

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

    // ONLY non-canonical read control behaviour
    settings.c_cc[VMIN]  = 1;     // min bytes to return read
    settings.c_cc[VTIME] = 30;    // read timeout in 1/10 seconds

    fcntl(
        fdin,
        F_SETFL,
        FNDELAY // return 0 if no chars available on port (non-blocking)
    ); // immediate reads

    tcflush(fdin, TCIFLUSH); // flush port before persisting changes

    int apply_settings = -1;
    if ( (apply_settings = tcsetattr(fdin, TCSANOW, &settings)) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nHE100_configureInterface: failed to set config: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int flush_device = -1;
    if ( (flush_device = tcsetattr(fdin, TCSAFLUSH, &settings)) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nHE100_configureInterface: failed to flush device: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nHE100_configureInterface: successfully applied new settings and flush device");
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
        // Could not open port
        fprintf(
            stderr,
            "\r\nHE100_openPort: Unable to open port: %s Errno:%s\n Line:%d", port_address, strerror(errno), __LINE__
        );
        return -1;
    }

    if ( !isatty(fdin) ) {
        fprintf(
            stderr,
            "\r\nHE100_openPort: Not a serial device: %s Errno: %s\n Line:%d\r\n", port_address, strerror(errno), __LINE__
        );
        return -1;
    }

    fprintf(stdout, "\r\nSuccessfully opened port: %s",port_address);
    HE100_configureInterface(fdin);

    return(fdin);
}

/* Function to close serial device connection at given file descriptor */
int
HE100_closePort(int fdin)
{
    if (close(fdin) == -1)
    {
        fprintf(
            stderr,
            "\r\nHE100_closePort: Unable to close the serial device connection! Line:%d", __LINE__
        );
        return -1;
    }
    fprintf(stdout, "\r\nHE100_closePort: Closed device %s successfully!\r\n",port_address);
    return 1;
}

/**
 * Function to write a given byte sequence to the serial device
 * @param fdin - the file descriptor representing the serial device
 * @param bytes - the char array containing the byte sequence to write
 * @param size - the length of the array in bytes
 */
int
HE100_write(int fdin, unsigned char *bytes, size_t size)
{
    // Output outgoing transmission
    //fprintf(stdout, "\r\nWriting to device: ");

    // Write byte array
    int w = write (fdin, bytes, size);
    int write_return = 1;

    //size_t j=0;
    //for (j=0; j<size; j++)
    //    printf("%02X ",bytes[j]);
    //fprintf(stdout, "\r\nWrite size: %d\n",w);

    fflush( NULL ); fsync(fdin); // TODO fdin, is a tty device, ineffective?

    // Issue a read to check for ACK/NOACK
    if ( HE100_read(fdin, 2) > 0 ) {
       write_return = 0;
    }
    if (w>0) {
        write_return = 0;
    }
    return write_return;
}

/**
 * Function to parse a given frame, validate it, and return its data
 * @param response - the frame data to be validated
 * @param length - the entire length of the frame in bytes
 */
//unsigned char*
int
HE100_storeValidResponse (unsigned char *response, size_t length)
{
    unsigned char *data = (unsigned char *) malloc(length);
    int r=0; // return value
    // prepare container for decoded data
    size_t data_length = length - 2; // response minus 2 sync bytes
    int hb1 = 6; int hb2 = 7;
    int pb1 = length-2; int pb2 = length-1;

    size_t payload_length = length - 10; // response minus header minus 4 checksum bytes and 2 sync bytes and 2 length bytes
    unsigned char *msg = (unsigned char *) malloc(data_length);

    // copy the header into the new response array minus sync bytes
    size_t i; size_t j=0;
    for (i=2;i<8;i++) {
        data[j] = response[i];
        j++;
    }
    HE100_dumpHex(stdout,data,4);

    // generate and compare header checksum
    fletcher_checksum h_chksum = fletcher_checksum16(data,4);
    uint8_t h_s1_chk = memcmp(&response[hb1], &h_chksum.sum1, 1);
    uint8_t h_s2_chk = memcmp(&response[hb2], &h_chksum.sum2, 1);
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

    if (response[4] == response[5] ) /* ACK or NOACK or EMPTY length */
    {
        if (response[4] == 10) {
            // TODO log with shakespeare
            //fprintf(stdout,"\r\n  HE100: Acknowledge");
            /* TODO Check the header checksum here, a bit different than payload responses */
            r = 0;
        } else if (response[4] == 255) {
            // TODO log with shakespeare
            printf("\r\n  HE100: No-Acknowledge");
            r = 1;
        } else {
            // TODO log with shakespeare
            printf("\r\n  HE100: Empty length? \r\n");
            r = 1;
        }
    }
    else
    {
        if (h_chk != 0) {
            fprintf(stdout,"\r\nInvalid header checksum \r\n    Incoming: [%d,%d] Calculated: [%d,%d]",(uint8_t)response[hb1],(uint8_t)response[hb2],(uint8_t)h_chksum.sum1,(uint8_t)h_chksum.sum2);
            // DISABLED FOR TESTING, TODO, FIX! r=-1;
        }

        if (p_chk != 0) {
            fprintf(stdout,"\r\nInvalid payload checksum \r\n   Incoming: [%d,%d] Calculated: [%d,%d]",(uint8_t)response[pb1],(uint8_t)response[pb2],(uint8_t)p_chksum.sum1,(uint8_t)p_chksum.sum2);
            // DISABLED FOR TESTING, TODO, FIX! // r=-1;
        }

        j=0; // fill payload message array
        for (i=10;i<length;i++) {
            msg[j] = response[i];
            j++;
        }
    }

    if (r==0) {
        pipe_init();
        datapipe.WriteToPipe(msg, payload_length);
        //return (char*) msg; // return the stripped message so it doesn't have to be done again
        // TODO we aren't doing this because we want the incoming messages to collect even if the the netman is failing to act on them properly. Is this reason enough to continue in this way?
    }

    free(data);
    free(msg);
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
 */
int
HE100_read (int fdin, time_t timeout)
{
    // Read response
    unsigned char buffer[1];
    unsigned char response[255];
    int i=0;
    int r=0; // return value for HE100_read
    int breakcond=255;
    timer_t read_timer = timer_get();
    timer_start(&read_timer,timeout,0);
    // Variables for select
    int ret_value;

    // Read continuously from serial device
    //signal(SIGINT, inthand);

    struct pollfd fds;
    fds.fd = fdin;
    fds.events = POLLIN;

    while (!timer_complete(&read_timer))
    {
        if ( ( ret_value = poll(&fds, 1, 5) ) /* TODO not nice, be explicit */ ) // if a byte is read
        {
	        read(fdin, &buffer, 1);

            // set break condition based on incoming byte pattern
            if ( i==4 && (buffer[0] == 0x0A || buffer[0] == 0xFF ) ) // getting an ack
            {
                breakcond=8; // ack is 8 bytes
            }
            else if ( i==5 && breakcond==255 ) // this is the length byte, set break point accordingly
            { // could be done by HE100_referenceByteSequence
                breakcond = buffer[0] + 10;
            }

            // increment response array values based on byte pattern
            if ( HE100_referenceByteSequence(buffer,i) > 0 )
            {
                    response[i]=buffer[0];
                    buffer[0] = '\0';
                    i++;
            }
            else
            {
                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
            }

            if (i==breakcond)
            {
                if (i>0) // we have a message to validate
                {
                    if ( HE100_storeValidResponse(response, breakcond) == 0 )
                    {
                        fprintf(stdout, "\r\n VALID MESSAGE!\r\n");
                        r = 1; // we got a frame, time to ack!
                    }
                    else
                    {
                        fprintf(stderr, "\r\n Invalid data!\r\n");
                        r=-1;
/*  TODO
                        // soft reset the transceiver
                        if ( HE100_softReset(fdin) > 0 )
                            printf("\r\n Soft Reset written successfully!");
                        else
                            printf("\r\n Problems with soft reset");
*/
                    }
                }

                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
            }
            buffer[0] = '\0'; // wipe buffer each time
        }
        else if (ret_value == -1)
        {
            // bad or no read
	        printf("Oh dear, something went wrong with select()! %s\n", strerror(errno));
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
unsigned char*
HE100_prepareTransmission(unsigned char *payload, size_t length, unsigned char *command)
{
    size_t transmission_length;
    size_t payloadbytes_length;
    size_t i; // payload index
    int payload_chksum_bool;

    // set the array bounds based on command
    if (command[1] == 0x01 || command[1] == 0x02 || command[1] == 0x12 || command[1] == 0x05) /* empty payload */ {
        transmission_length = 8;
        payloadbytes_length = 6;
        payload_chksum_bool = 0;
    } else {
        transmission_length = length+10;
        payloadbytes_length = length+8;
        payload_chksum_bool = 1;
    }

    unsigned char *transmission = (unsigned char *) malloc(transmission_length); // TODO free me!
    unsigned char *payloadbytes = (unsigned char *) malloc(payloadbytes_length);

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
    int r = -1;
    switch ((int)position)
    {
        case 0   : // first position should be 0x48 : 72
                if ((int)*response==72) r=1;
                break;
        case 1   : // second position should be 0x65 : 101
                if ((int)*response==101) r=1;
                break;
        case 2   : // response tx/rx command should be 0x20
                if ((int)*response==32) r=1; // CMD_RECEIVE  0x20
                break;
        case 3   : // response command could be between 1-20
                if (*response > 0x00 && *response <= 0x20) r=(int)*response;
                break;
        case 4   : // first length byte
                if (
                       *response == 0x00 // start of length byte
                    || *response == 0x0A // start of ack, should match 5
                    || *response == 0xFF // start of noack, should match 5
                   ) r=1;
                break;
        //case 5   : // real length byte
                //if ((int)*response<MAX_FRAME_LENGTH) r=(int)*response;
                //break;
        default  : r=1; break;
    }
    return r;
}

/**
 * Function to decode validated and extracted data from response
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

// TODO these settings are not clearly defined in documentation and need to be confirmed
struct he100_settings
HE100_prepareConfig (unsigned char * buffer)
{
    he100_settings settings; 
    //if ( (int)buffer[5] == CFG_PAYLOAD_LENGTH) 
    //{ // check that we have the expected CFG payload length
      // pour data into struct
      settings.interface_baud_rate = buffer[10+0]; 
      settings.tx_power_amp_level = buffer[10+1]; 
      settings.rx_rf_baud_rate = buffer[10+2]; 
      settings.tx_rf_baud_rate = buffer[10+3]; 
      settings.rx_modulation = buffer[10+4]; 
      settings.tx_modulation = buffer[10+5]; 
      settings.rx_freq = buffer[10+6]; 
      //settings.rx_freq = buffer[10+7]; 
      settings.tx_freq = buffer[10+10]; 
      //settings.tx_freq = buffer[10+11]; 
     
      //unsigned char * scl[7];
      //unsigned char * dcl[7];
      //memcpy(scl,buffer+10+14,6);
      memcpy(settings.source_callsign,(unsigned char*)buffer+10+14,6);
      //memcpy(dcl,buffer+10+20,6);
      memcpy(settings.destination_callsign,(unsigned char*)buffer+10+20,6);

      settings.tx_preamble = buffer[10+26]; 
      settings.tx_postamble = buffer[10+28]; 

      //settings.dio_pin14 = buffer[10+30]; 
      //settings.rx_crc = buffer[10+30]; 

      settings.rxtx_test_cw = buffer[10+32]; 
      settings.ext_conf_setting = buffer[10+33]; 
      settings.led_blink_type = buffer[10+38]; 
    //}

    return settings;
}

// TODO redundant to call two structs. What do we want to DO with these settings? Perhaps write to file.
struct he100_settings 
HE100_getConfig (int fdin)
{//48 65 10 05 00 00 15 4F
    unsigned char get_config_payload[1] = {0};
    unsigned char get_config_command[2] = {CMD_TRANSMIT, CMD_GET_CONFIG};
    
    struct he100_settings old_settings;

    // not ready yet, return empty
    return old_settings;
    
    if(
        HE100_write(
            fdin,
            HE100_prepareTransmission(get_config_payload, 0, get_config_command),
            EMPTY_PAYLOAD_WRITE_LENGTH
        ) > 0
    )
    {
       if ( HE100_read(fdin, 1) ) 
       {
         unsigned char buffer[44];
         datapipe.ReadFromPipe((char*)buffer, CFG_FRAME_LENGTH);
         if ( HE100_referenceByteSequence(buffer,3) == 0x05 ) // check if is config
         {
            he100_settings settings = HE100_prepareConfig(buffer); 
            return settings;   
         }
         else return old_settings;
       }
    }
}

/**  
 *  This function validates a he100_settings struct
 *  and converts to char array for safe deployment
 **/ 
// TODO currently idea is to set a value in the array that is impossible to 
// represent a false result. Clearly there is a better way to do this     
unsigned char * 
HE100_validateConfig (struct he100_settings he100_new_settings)
{ 
    // initiate config payload array
    unsigned char * set_config_payload = (unsigned char *) malloc (CFG_PAYLOAD_LENGTH);

    // validate new interface baud rate setting
    if (
            he100_new_settings.interface_baud_rate < MAX_IF_BAUD_RATE 
        &&  he100_new_settings.interface_baud_rate > MIN_IF_BAUD_RATE 
        &&  he100_new_settings.interface_baud_rate != CFG_DEF_IF_BAUD 
    ) 
    {
         // TODO this SHOULD changed with the serial connection as well?
         set_config_payload[CFG_IF_BAUD_BYTE] = he100_new_settings.interface_baud_rate;
    }
    //else set_config_payload[CFG_IF_BAUD_BYTE]=-1;
    else set_config_payload[0]=255;

    // validate new power amplification level
    if (
            he100_new_settings.tx_power_amp_level > MIN_PA_LEVEL 
            && he100_new_settings.tx_power_amp_level < MAX_PA_LEVEL
       ) 
    {
        set_config_payload[CFG_PA_BYTE] = he100_new_settings.tx_power_amp_level;
    } 
    //else set_config_payload[CFG_PA_BYTE]=-1;
    else set_config_payload[0]=255;

    // validate new rf baud rates
    if (
            he100_new_settings.rx_rf_baud_rate < MAX_RF_BAUD_RATE
        &&  he100_new_settings.rx_rf_baud_rate > MIN_RF_BAUD_RATE
        &&  he100_new_settings.tx_rf_baud_rate < MAX_RF_BAUD_RATE
        &&  he100_new_settings.tx_rf_baud_rate > MIN_RF_BAUD_RATE
    ) 
    {
        set_config_payload[CFG_RF_RX_BAUD_BYTE] = he100_new_settings.rx_rf_baud_rate;
        set_config_payload[CFG_RF_TX_BAUD_BYTE] = he100_new_settings.tx_rf_baud_rate;
    }
    else {
        set_config_payload[CFG_RF_RX_BAUD_BYTE]=-1;
        set_config_payload[CFG_RF_TX_BAUD_BYTE]=-1;
    }
   
    // validate modulation (USE DEFAULTS)
    if (
            he100_new_settings.rx_modulation == CFG_RX_DEF_MOD
         && he100_new_settings.tx_modulation == CFG_TX_DEF_MOD        
    )
    {
        set_config_payload[CFG_RX_MOD_BYTE] = he100_new_settings.rx_modulation;
        set_config_payload[CFG_TX_MOD_BYTE] = he100_new_settings.tx_modulation;
    }
    else set_config_payload[0]=255;

    // validate new RX setting
    if ( 
        (he100_new_settings.rx_freq > MIN_UPPER_FREQ && he100_new_settings.rx_freq < MAX_UPPER_FREQ)
     || (he100_new_settings.rx_freq > MIN_LOWER_FREQ && he100_new_settings.rx_freq < MAX_LOWER_FREQ)
    )
    {
       // don't know how to set this yet
    }
    else set_config_payload[0]=255;

    // validate new TX setting
    if ( 
        (he100_new_settings.tx_freq > MIN_UPPER_FREQ && he100_new_settings.tx_freq < MAX_UPPER_FREQ)
     || (he100_new_settings.tx_freq > MIN_LOWER_FREQ && he100_new_settings.tx_freq < MAX_LOWER_FREQ)
    )
    {
       // don't know how to set this yet
    }
    else set_config_payload[0]=255;

    // validate callsigns (USE DEFAULTS) 
    if ( 1
        // length should be 6, valid CALLSIGN
        //( memcmp(he100_new_settings.source_callsign, CFG_SRC_CALL_DEF) == 0 )
     //&& ( memcmp(he100_new_settings.destination_callsign, CFG_DST_CALL_DEF) == 0 )
    )
    {
        int i;
        int j=0;
        //he100_new_settings.source_callsign = CFG_SRC_CALL_DEF;
        //unsigned char SRC_CALL[7] = "VA3ORB";
        for (i=CFG_SRC_CALL_BYTE;i<CFG_SRC_CALL_BYTE+6;i++)
            set_config_payload[i] = CFG_SRC_CALL_DEF[j];

        //he100_new_settings.destination_callsign = CFG_DST_CALL_DEF;
        //unsigned char DST_CALL[7] = "VE2CUA";
        j=0;
        for (i=CFG_DST_CALL_BYTE;i<CFG_DST_CALL_BYTE+6;i++)
            set_config_payload[i] = CFG_DST_CALL_DEF[j];
    }
    else set_config_payload[0]=255;

  
/***   DIO PIN 13, RX CRC, LED, this doesn't make much sense yet:  *******************/
/* the following settings are in conflict because they are set by the same byte */
/* CFG_RX_CRC_BYTE == CFG_LED_BYTE == CFG_DIO_PIN13_BYTE == 30 */
/* 0x43 could conceivably simultaneously enable CFG_RX_CRC_ON, CFG_LED_RX, and CFG_DIO_PIN13_OFF */

    // validate RX CRC setting 
    if (
            he100_new_settings.rx_crc == CFG_RX_CRC_ON
         || he100_new_settings.rx_crc == CFG_RX_CRC_OFF
        )
    {
        set_config_payload[CFG_RX_CRC_BYTE] = he100_new_settings.rx_crc;
    }
    else set_config_payload[0]=255;

    // validate new LED setting
    if (
            he100_new_settings.led_blink_type == CFG_LED_PS
         || he100_new_settings.led_blink_type == CFG_LED_RX
         || he100_new_settings.led_blink_type == CFG_LED_TX  
        )
    {
        set_config_payload[CFG_LED_BYTE] = he100_new_settings.led_blink_type;
    }
    else set_config_payload[0]=255;

    // validate new LED setting
    if (
            he100_new_settings.dio_pin13 == CFG_DIO_PIN13_OFF
         || he100_new_settings.dio_pin13 == CFG_DIO_PIN13_TXRXS
         || he100_new_settings.dio_pin13 == CFG_DIO_PIN13_2p5HZ 
         || he100_new_settings.dio_pin13 == CFG_DIO_PIN13_RXTOG
        )
    {
        set_config_payload[CFG_DIO_PIN13_BYTE] = he100_new_settings.dio_pin13;
    }
    else set_config_payload[0]=255;

/*************************************************************************************/
    // validate TX Test CW (USE DEFAULTS) 
    if ( he100_new_settings.rxtx_test_cw == CFG_RXTX_TEST_CW_DEF )
    {
        set_config_payload[CFG_RXTX_TEST_CW_BYTE] = he100_new_settings.rxtx_test_cw;
    }
    else set_config_payload[0]=255;

    // validate EXT functions
    switch (he100_new_settings.ext_conf_setting)
    {
        case 0 : /* all EXT functions off */
           set_config_payload[CFG_EXT_BYTE] = 0;
           break; 
        case CFG_EXT_PING_ON: /* ping on */        
           set_config_payload[CFG_EXT_BYTE] = CFG_EXT_PING_ON;
           break; 
        case CFG_EXT_CODEUPLOAD_ON: /* code upload on */ 
           set_config_payload[CFG_EXT_BYTE] = CFG_EXT_CODEUPLOAD_ON;
           break; 
        case CFG_EXT_RESET_ON: /* reset on */
           set_config_payload[CFG_EXT_BYTE] = CFG_EXT_RESET_ON;
           break; 
        default : 
           set_config_payload[CFG_EXT_BYTE] = CFG_EXT_DEF;
           break;
    } // if no valid option, go with default

    return set_config_payload;
}

/**
 *  Function to persist a given he100_settings struct after it passes validation
 */   
int 
HE100_setConfig (int fdin, struct he100_settings he100_new_settings)
{
    fprintf(stdout, "config function not yet implemented, try again later");
    return -1;

    unsigned char *set_config_payload = HE100_validateConfig(he100_new_settings);
    unsigned char set_config_command[2] = {CMD_TRANSMIT, CMD_SET_CONFIG};

    // TODO expand to check all parameters and display all errors present 
    if (set_config_payload[0] != 255) // we have valid array
        if(
            HE100_write(
                fdin,
                HE100_prepareTransmission(set_config_payload, CFG_PAYLOAD_LENGTH, set_config_command),
                CFG_PAYLOAD_LENGTH+10
            ) > 0
        )
            free(set_config_payload);
            return 1; // Successfully wrote config
        
    free(set_config_payload);
    return -1; // failed to set and/or write config
}

int 
HE100_writeFlash (int fdin, unsigned char *flash_md5sum, size_t length)
{
    // not ready yet
    return -1;

    HE100_dumpHex (stdout,flash_md5sum,24); // TODO handle this properly, see docs

    size_t write_length = length + 10;
    unsigned char *write_flash_payload = (unsigned char *) malloc(length);
    unsigned char write_flash_command[2] = {CMD_TRANSMIT, CMD_WRITE_FLASH};
    
    if(
        HE100_write(
            fdin,
            HE100_prepareTransmission(write_flash_payload, length, write_flash_command),
            write_length
        ) > 0
    )
        return 1; // Successfully wrote config
        
    return -1; // failed to set and/or write config
}
