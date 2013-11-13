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
#include "./SC_he100.h" /*  Header file that exposes the correct serial device location */
#include "he100.h"      /*  Header file that exposes the correct serial device location */

#define LOG_FILE_PATH "/var/log/he100/he100.log"
#define DATA_PIPE_PATH "/var/log/he100/data.log"

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

// Config options
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
#define CFG_RX_MOD_BYTE     
#define CFG_RX_DEF_MOD      0x00 
#define CFG_RX_MOD_BYTE     
#define CFG_TX_DEF_MOD      0x00
// LED config
#define CFG_LED_BYTE        30  // 31st byte
#define CFG_LED_PS          0x41 // 2.5 second pulse
#define CFG_LED_TX          0x42 // flash on transmit
#define CFG_LED_RX          0x43 // flash on receive

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

// RX CRC config
//#define CFG_RX_CRC_BYTE     30 // 31st byte is for LED, no?
//#define CFG_RX_CRC_ON       0x43
//#define CFG_RX_CRC_OFF      0x03

/**
 * Function to configure interface
 * @param fdin - the file descriptor representing the serial device
 * REF: http://man7.org/linux/man-pages/man3/termios.3.html
 * REF: http://www.unixguide.net/unix/programming/3.6.2.shtml
 */
void
HE100_configureInterface (int fdin)
{ 
    struct termios settings;
   
    // get current settings
    int get_settings = -1; 
    if ( get_settings = tcgetattr(fdin, &settings) < 0 ) 
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
/*    
    settings.c_oflag &= ~(
          OCRNL  // turn off output processing
        | ONLCR  // no CR to NL translation 
        | ONLRET // no NL to CR-NL translation
        | ONOCR  // no NL to CR translation
        | ONOEOT // no column 0 CR suppression
        | OFILL  // no Ctrl-D suppression, no fill characters,
        | OLCUC  // no case mapping
        | OPOST  // no local output processing
    );
*/

    // Line processing flags
    settings.c_lflag = 0; // disable line processing 
    
    // settings.c_lflag = ICANON; // enable canonical input
    //cfmakeraw(&settings); // raw mode Input is not assembled into lines and special characters are not processed.
    
    settings.c_lflag = ECHONL; 
/*  
    settings.c_lflag &= ~(
          ECHO   // echo off
        | ECHONL // echo newline off
        | ICANON // canonical mode off
        | IEXTEN // extended input processing off
        | ISIG   // signal chars off
    );
*/

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

    //   VEOF, VEOL, VERASE, VKILL (and also 
    //   VEOL2, VSTATUS and VWERASE if defined and IEXTEN is set) 
    
/* 
    // ONLY canonical mode parameters
    settings.c_cc[VINTR]    = 0;     //  Ctrl-c  
    settings.c_cc[VQUIT]    = 0;     //  Ctrl-\
    settings.c_cc[VERASE]   = 0;     //  del 
    settings.c_cc[VKILL]    = 0;     //  @ 
    settings.c_cc[VEOF]     = 4;     //  Ctrl-d 
    settings.c_cc[VTIME]    = 0;     //  inter-character timer unused 
    settings.c_cc[VMIN]     = 1;     //  blocking read until 1 character arrives 
    settings.c_cc[VSWTC]    = 0;     //  '\0' 
    settings.c_cc[VSTART]   = 0;     //  Ctrl-q 
    settings.c_cc[VSTOP]    = 0;     //  Ctrl-s 
    settings.c_cc[VSUSP]    = 0;     //  Ctrl-z 
    settings.c_cc[VREPRINT] = 0;     //  Ctrl-r 
    settings.c_cc[VDISCARD] = 0;     //  Ctrl-u 
    settings.c_cc[VWERASE]  = 0;     //  Ctrl-w 
    settings.c_cc[VLNEXT]   = 0;     //  Ctrl-v 
    settings.c_cc[VEOL]     = 0;     //  end-of-line '\0' 
    settings.c_cc[VEOL2]    = 0;     //  end-of-line '\0' 
*/

    // only if ISIG is set:
    //   VINTR, VQUIT, VSUSP (and also VDSUSP if 
    //   defined and IEXTEN is set) 
    
    // only if IXON or IXOFF is set:
    //   VSTOP, VSTART 
   
    fcntl(
        fdin, 
        F_SETFL,  
        FNDELAY // return 0 if no chars available on port (non-blocking)
    ); // immediate reads
    
    tcflush(fdin, TCIFLUSH); // flush port before persisting changes 
    
    int apply_settings = -1;
    if ( apply_settings = tcsetattr(fdin, TCSANOW, &settings) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nHE100_configureInterface: failed to set config: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);           
    }
/*   
    int flush_device = -1;
    if ( flush_device = tcsetattr(fdin, TCSAFLUSH, &settings) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nHE100_configureInterface: failed to flush device: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);           
    }
*/
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
            "\r\nHE100_openPort: Unable to open port: %s", port_address, "%s\n", strerror(errno)
        );
        return -1;
    }
    
    if ( !isatty(fdin) ) {
        fprintf(
            stderr, 
            "\r\nHE100_openPort: Not a serial device: %s", port_address, "%s\n", strerror(errno)
        );
        return -1;
    }

    fprintf(stderr, "\r\nSuccessfully opened port: %s",port_address);
    HE100_configureInterface(fdin);

    return(fdin);
}

/* Function to close serial device connection at given file descriptor */
HE100_closePort(int fdin)
{
    if (close(fdin) == -1) 
    {
        fprintf(
            stderr, 
            "\r\nHE100_closePort: Unable to close the serial device connection!"
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
    fprintf(stdout, "\r\nWriting to device: ");

    // Write byte array
    int w = write (fdin, bytes, size); // write given 10 bytes to fdin 
    int j=0;
    for (j=0; j<size; j++) 
    {
        printf("%02X ",bytes[j]);
    }    
    fprintf(stdout, "\r\nWrite size: %d",w);

    //fflush(fdin);

    // Issue a read to check for ACK/NOACK
    HE100_read(fdin, 1);

    if (w>0) {
        return 1;
    } 
}

/**
 * struct to hold values of fletcher checksum
 */
typedef struct HE100_checksum {
    uint8_t sum1;
    uint8_t sum2;
} HE100_checksum;

/** 
 * 16-bit implementation of the Fletcher Checksum
 * returns two 8-bit sums
 * @param data - uint8_t const - data on which to perform checksum
 * @param bytes - size_t - number of bytes to process
 * inspired by http://en.wikipedia.org/wiki/Fletcher%27s_checksum#Optimizations
 */
struct HE100_checksum 
HE100_fletcher16unef (char *data, size_t bytes)
{
    uint8_t sum1=0, sum2=0;
    int i=0;

    for (i=0; i<bytes; i++) {
        // calculate sums
        sum1 = (sum1 + (data[i] % 255));
        sum2 += sum1;
        // Reduce to 8-bit
        //printf ("\r\ni:%d d:%d 1:%d 2:%d ",i,(char)data[i],sum1,sum2);
    }

    // prepare and return checksum values 
    HE100_checksum r;
    r.sum1 = (sum1 % 255);
    r.sum2 = (sum2 % 255);
    return r;
}

/** Optimized Fletcher Checksum  
 * 16-bit implementation of the Fletcher Checksum
 * returns two 8-bit sums
 * @param data - uint8_t const - data on which to perform checksum
 * @param bytes - size_t - number of bytes to process
 * inspired by http://en.wikipedia.org/wiki/Fletcher%27s_checksum#Optimizations
 */
struct HE100_checksum
HE100_fletcher16 (char *data, size_t bytes)
{
    uint8_t sum1 = 0, sum2 = 0;

    while (bytes)
    {
        size_t tlen = bytes > 20 ? 20 : bytes; 
        bytes -= tlen;
        do
        {
            sum2 += sum1 += *data++;
        }
        while (--tlen);
    }
    
    // prepare and return checksum values 
    HE100_checksum r;
   
    // final reduction to 8 bits
    r.sum1 = (sum1 & 0xff);
    r.sum2 = (sum2 & 0xff);
    
    return r;
}

/**
 * Function to parse a given frame, validate it, and return its data
 * @param response - the frame data to be validated 
 * @param length - the entire length of the frame in bytes
 */
//unsigned char*
int
HE100_validateResponse (char *response, size_t length) 
{
    fprintf(stdout,"\r\n  HE100_validateResponse: validating %d byte message",(int)length);
    unsigned char *data = (char *) malloc(length);
    int r=1;

    // prepare container for decoded data
    int data_length = length - 10; // response minus header minus 4 checksum bytes
    unsigned char *msg = (char *) malloc(data_length);

    int i; int j=0;
    for (i=2;i<7;i++) 
        data[j] = response[i];
    
    // generate and compare header checksum
    HE100_checksum h_chksum = HE100_fletcher16(data,10); 
    int h_s1_chk = memcmp(&response[8], &h_chksum.sum1, 1);
    int h_s2_chk = memcmp(&response[9], &h_chksum.sum2, 1);
    int h_chk = h_s2_chk + h_s2_chk; // should be zero given valid chk

    // pick up j where it left off
    for (i=8;i<length-2;i++) // read up to, not including, payload chksums
        data[j] = response[i];

    // generate and compare payload checksum
    //HE100_checksum p_chksum = HE100_fletcher16(data,10); 
    HE100_checksum p_chksum = HE100_fletcher16(data,length-2); // chksum everything except 'He'
    int p_s1_chk = memcmp(&response[length-2], &p_chksum.sum1, 1);
    int p_s2_chk = memcmp(&response[length-1], &p_chksum.sum2, 1);
    int p_chk = p_s1_chk + p_s2_chk; // should be zero given valid chk
    
    if (response[4] == response[5] ) // ACK or NOACK or EMPTY length
    {
        if (response[4] == 10) 
        {
            // log with shakespeare
            fprintf(stdout,"\r\n  HE100: Acknowledge");
        }
        else if (response[4] = 255) 
        {
            // log with shakespeare
            printf("\r\n  HE100: No-Acknowledge");

        }
        else 
        {
            // log with shakespeare
            printf("\r\n  HE100: Empty length?");
        } 
    } 
    else 
    {
        if (h_chk != 0) {
            fprintf(stdout,"\r\nInvalid header checksum");
            r=-1;
        }
        if (p_chk != 0) {
            fprintf(stdout,"\r\nInvalid payload checksum");
            r=-1;
        }
        
        printf("\r\nMessage: ");
        j=0;
        for (i=10;i<data_length;i++)
            fprintf(stdout,"%s",(char*)&response[i]);
            msg[j] = response[i];

        if (r==1) 
        {
            //dump contents to helium data storage pipe
            //fdata = fopen(DATA_PIPE_PATH,"a");
            //HE100_dumpBytes(fdata, msg, data_length);
            //fclose(fdata);
            //return (char*) msg; 
        }
    }

    return r;
}

/* Function to dump a given array to a given file descriptor */
int
HE100_dumpBytes(FILE *fdout, unsigned char *bytes, size_t size) 
{
    // Output outgoing transmission
    int j=0;
    for (j=0; j<size; j++) 
    {
        fprintf(fdout,"%02X ",bytes[j]);
    }    
    return 1;
}

/**
 * Function to read bytes in single-file from the serial device and 
 * append them to and return a response array
 * 
 * @param fdin - the file descriptor representing the serial device
 */
int
HE100_read (int fdin, time_t timeout) 
{
    //FILE *fdout; 
    //fdout = fopen("/var/log/space/he100.log","a");

    // Read response
    unsigned char buffer[1];
    unsigned char response[255];
    int chars_read;
    int i=0;
    int action=0;
    int breakcond=255;
    
    timer_t read_timer = (timer_t)timer_get();
    timer_start(&read_timer,timeout);

    while (!timer_complete(&read_timer))
    {
        if ( chars_read = read(fdin, &buffer, 1) > 0 ) // if a byte is read
        { 
            printf("\r\n HE100_read: i:%d chars_read:%d buffer:0x%02X",i,chars_read,buffer[0]);

            // set break condition based on incoming byte pattern
            if ( i==4 && (buffer[0] == 0x0A || buffer[0] == 0xFF ) ) // getting an ack
            {
                breakcond=8; // ack is 8 bytes
            }
            else if ( i==5 && breakcond==255 ) // this is the length byte, set break point accordingly
            {
                fprintf(stdout,"\r\n HE100_read: Got length byte");
                breakcond = buffer[0] + 10;
            }

            if ( HE100_referenceByteSequence(buffer[0],i) > 0 ) 
            {
                    fprintf(stdout," >> returned 1");
                    response[i]=buffer[0];
                    buffer[0] = '\0';
                    //fprintf(fdout, "0x%02X ", response[i]);
                    fprintf(
                        stdout,
                        "\n  i:%d breakcondition:%d",
                        i,breakcond
                    );
                    i++;
            }
            else 
            {
                fprintf(stdout," >> returned -1");
                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
            }

            if (i==breakcond) 
            {
                fprintf(stdout,"\n HE100_read: hit break condition!");
                if (i>0) // we have a message to validate 
                {
                    if ( HE100_validateResponse(response, breakcond) > 0 ) 
                    {
                        fprintf(stdout, "\r\n VALID MESSAGE!");
                        return 1; // we got a frame, time to ack!
                    }
                    else 
                    {
                        fprintf(stderr, "\r\n Invalid data!\r\n");
                        HE100_dumpBytes(stdout, response, i+1);            
                        // soft reset the transceiver 
                        size_t write_len = EMPTY_PAYLOAD_WRITE_LENGTH;
                        if ( HE100_write(fdin, HE100_softReset(), write_len) > 0 )
                            printf("\r\n Soft Reset written successfully!");
                        else  
                            printf("\r\n Problems writing to serial device");
                    }
                }

                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
                //fprintf(fdout,"\n");
            }

            buffer[0] = '\0'; // wipe buffer each time
        }
        else if (chars_read = -1) 
        {
            return chars_read;
        }
    }
    // received nothing
    return 0;
}

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of data stream
 */
unsigned char* 
HE100_prepareTransmission(
    unsigned char *payload, 
    size_t length, 
    unsigned char *command
)
{
    unsigned char *transmission = (char *) malloc(length+10);
    unsigned char *payloadbytes = (char *) malloc(length+8);
    
    // attach sync bytes to final transmission byte array
    transmission[0] = SYNC1; //0x48;
    transmission[1] = SYNC2; //0x65;

    // attach command bytes to intermediary payload byte array
    payloadbytes[0] = (unsigned char) command[0];
    payloadbytes[1] = (unsigned char) command[1];

    // attach length bytes
    payloadbytes[2] = 0x00;
    payloadbytes[3] = (unsigned char) length & 0xff; 

    // generate and attach header checksum
    HE100_checksum header_checksum = HE100_fletcher16(payloadbytes,4); 
    payloadbytes[4] = (unsigned char) header_checksum.sum1 & 0xff;
    payloadbytes[5] = (unsigned char) header_checksum.sum2 & 0xff;
    printf ("\r\nheader_checksum: [%d,%d], [%d,%d]",
        header_checksum.sum1, header_checksum.sum2,
        payloadbytes[4], payloadbytes[5]);
    
    // generate and attach payload checksum
    HE100_checksum payload_checksum = HE100_fletcher16(payload,length); // chksum only payload
    //HE100_checksum payload_checksum = HE100_fletcher16(payloadbytes,length+6); // chksum everything except 'He'
    payloadbytes[6+length] = payload_checksum.sum1;
    payloadbytes[6+length+1] = payload_checksum.sum2;
    printf ("\r\npayload_checksum: [%d,%d], [%d,%d]",
        payload_checksum.sum1, payload_checksum.sum2,
        payloadbytes[6+length], payloadbytes[6+length+1]);

    // attach data to payload 
    int i;
    for (i=0;i<length;i++)
        payloadbytes[6+i] = payload[i];
    int j=0;

    // attach payload and return final transmission
    for (i=2;i<length+10;i++) {
        transmission[i] = payloadbytes[j];
        j++;
    }

    return (char*) transmission;
}

int
HE100_referenceByteSequence(unsigned char *response, int position)
{
    printf("\r\n  HE100_referenceByteSequence(0x%02X,%d)",*response,(int)position);
    int r = -1;
    switch ((int)position)
    {
        case 0   : // first position should be 0x48 : 72
                if ((int)*response==72) r=1;
                //else if ((*response)==NULL) return -1;
                break;
        case 1   : // second position should be 0x65 : 101 
                if ((int)*response==101) r=1;
                break;
        case 2   : // response tx/rx command should be 0x20
                if ((int)*response==32) r=1; // CMD_RECEIVE  0x20
                break;
        case 3   : // response command could be between 1-20
                if (*response > 0x00 && *response < 0x20) r=(int)*response; 
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
HE100_interpretResponse (char *response, size_t length) 
{
    printf("Response length: %d\n", (int)length);

    // if response == transmission, He100 device is off!   
    if ((int)response[2]==16) {
        fprintf(stderr,"HE100_read: He100 is off!");
        return 0;
    }

    char* value; 
    int i=0;
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
unsigned char *
HE100_NOOP ()
{
   //noop[10] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43,0x00,0x00};
   unsigned char noop_payload[1] = {0};
   unsigned char noop_command[2] = {CMD_TRANSMIT, CMD_NOOP};
   return HE100_prepareTransmission(noop_payload, 0, noop_command);
}

/**
 * Function returning byte sequence to set the beacon message 
 * unsigned char *beacon_message_payload message to transmit 
 */
unsigned char *
HE100_transmitData (unsigned char *transmit_data_payload, size_t transmit_data_len)
{
   unsigned char transmit_data_command[2] = {CMD_TRANSMIT, CMD_TRANSMIT_DATA};
   return HE100_prepareTransmission(transmit_data_payload, transmit_data_len, transmit_data_command);
}

/**
 * Function returning byte sequence to enable beacon on given interval 
 * int beacon_interval interval in seconds 
 */
unsigned char *
HE100_setBeaconInterval (int beacon_interval)
{
   unsigned char beacon_interval_payload[1] = {beacon_interval};
   unsigned char beacon_interval_command[2] = {CMD_TRANSMIT, CMD_BEACON_CONFIG};
   return HE100_prepareTransmission(beacon_interval_payload, 1, beacon_interval_command);
}

/**
 * Function returning byte sequence to set the beacon message 
 * unsigned char *beacon_message_payload message to transmit 
 */
unsigned char *
HE100_setBeaconMessage (unsigned char *set_beacon_message_payload, size_t beacon_message_len)
{
   unsigned char set_beacon_message_command[2] = {CMD_TRANSMIT, CMD_BEACON_DATA};
   return HE100_prepareTransmission(set_beacon_message_payload, beacon_message_len, set_beacon_message_command);
}

/**
 * Function returning byte sequence to amplify power based
 * on input int power_level
 * int power_level decimal value from 0-255 (0%-100%)
 */
unsigned char *
HE100_fastSetPA (int power_level)
{
   unsigned char fast_set_pa_payload[1] = {power_level};
   unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
   return HE100_prepareTransmission(fast_set_pa_payload, 1, fast_set_pa_command);
}

/**
 * Function returning byte sequence to soft reset HE100 board and restore flash settings 
 * no arguments
 */
unsigned char *
HE100_softReset()
{
   unsigned char soft_reset_payload[1] = {0}; 
   unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
   return HE100_prepareTransmission(soft_reset_payload, 0, soft_reset_command);
}

/**
 * Function returning byte sequence to return firmware version 
 * no arguments
 */
unsigned char *
HE100_readFirmwareRevision()
{
   unsigned char read_firmware_revision_payload[1] = {0}; 
   unsigned char read_firmware_revision_command[2] = {CMD_TRANSMIT, CMD_READ_FIRMWARE_V};
   return HE100_prepareTransmission(read_firmware_revision_payload, 0, read_firmware_revision_command);
}

struct he100_settings HE100_getConfig (int fdin)
{
    unsigned char get_config_payload[1] = {0};
    unsigned char get_config_command[2] = {CMD_TRANSMIT, CMD_GET_CONFIG};
    
    struct he100_settings old_settings;

    // not ready yet, return empty
    return old_settings;
    
    if(
        SC_write(
            fdin,
            HE100_prepareTransmission(get_config_payload, 0, get_config_command),
            EMPTY_PAYLOAD_WRITE_LENGTH
        ) > 0
    )
    {
       if ( HE100_read(fdin, 1) ) 
       {
            // check if is config
            // data other than config could have been in buffer!
            // pour data into struct
       }
    }
}

int 
HE100_setConfig (int fdin, struct he100_settings he100_new_settings)
{
    int r = -1;
 
    // initiate config payload array
    unsigned char set_config_payload[22];
    unsigned char set_config_command[2] = {CMD_TRANSMIT, CMD_SET_CONFIG};

    // validate new interface baud rate setting
    if (
            he100_new_settings.interface_baud_rate < MAX_IF_BAUD_RATE 
        &&  he100_new_settings.interface_baud_rate > MIN_IF_BAUD_RATE 
        &&  he100_new_settings.interface_baud_rate != CFG_DEF_IF_BAUD 
    ) 
    {
        // this would have to be changed with the serial connection as well
        // setttings_array[CFG_IF_BAUD_BYTE] = he100_new_settings.interface_baud_rate;
        return -1;
    }

    // validate new power amplification level
    if (
            he100_new_settings.tx_power_amp_level > MIN_PA_LEVEL 
            && he100_new_settings.tx_power_amp_level < MAX_PA_LEVEL
       ) 
    {
        set_config_payload[CFG_PA_BYTE] = he100_new_settings.tx_power_amp_level;
    }

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
   
    // validate modulation (USE DEFAULTS)
    if (
            he100_new_settings.rx_modulation == CFG_RX_DEF_MOD
         && he100_new_settings.tx_modulation == CFG_TX_DEF_MOD        
    )
    {
        //set_config_payload
    }

    // validate new LED setting
    if (
            he100_new_settings.led_blink_type == CFG_LED_PS
         || he100_new_settings.led_blink_type == CFG_LED_RX
         || he100_new_settings.led_blink_type == CFG_LED_TX  
        )
    {
        set_config_payload[CFG_LED_BYTE] = he100_new_settings.led_blink_type;
    }

    // validate new RX setting
    if ( 
        (he100_new_settings.rx_freq > MIN_UPPER_FREQ && he100_new_settings.rx_freq < MAX_UPPER_FREQ)
     || (he100_new_settings.rx_freq > MIN_LOWER_FREQ && he100_new_settings.rx_freq < MAX_LOWER_FREQ)
    )
    {
       // don't know how to set this yet
    }

    // validate new TX setting
    if ( 
        (he100_new_settings.tx_freq > MIN_UPPER_FREQ && he100_new_settings.tx_freq < MAX_UPPER_FREQ)
     || (he100_new_settings.tx_freq > MIN_LOWER_FREQ && he100_new_settings.tx_freq < MAX_LOWER_FREQ)
    )
    {
       // don't know how to set this yet
    }

    // not completed yet, so
    return -1;

    if (r==1) //
        if(
            SC_write(
                fdin,
                HE100_prepareTransmission(set_config_payload, CFG_PAYLOAD_LENGTH, set_config_command),
                CFG_PAYLOAD_LENGTH+10
            ) > 0
        )
            return 1; // Successfully wrote config
        
    return -1; // failed to set and/or write config
}

int 
HE100_writeFlash (int fdin, unsigned char *flash_md5sum, size_t length)
{
    // not ready yet
    return -1;

    size_t write_length = length + 10;
    unsigned char *write_flash_payload = (char *) malloc(length);
    unsigned char write_flash_command[2] = {CMD_TRANSMIT, CMD_WRITE_FLASH};
    
    if(
        SC_write(
            fdin,
            HE100_prepareTransmission(write_flash_payload, length, write_flash_command),
            write_length
        ) > 0
    )
        return 1; // Successfully wrote config
        
    return -1; // failed to set and/or write config
}
