/*
 * =====================================================================================
 *
 *       Filename:  SC_transceiverLib.c
 *
 *    Description:  Connect to a serial device
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
#include <stdio.h>   /*  Standard input/output definitions */
#include <stdint.h>  /*  Standard integer types */
#include <string.h>  /*  String function definitions */
#include <unistd.h>  /*  UNIX standard function definitions */
#include <fcntl.h>   /*  File control definitions */
#include <errno.h>   /*  Error number definitions */
#include <termios.h> /*  POSIX terminal control definitions */
#include "he100.h" 
#include "./he100_cfg.h"

int
SC_openPort(void)
{
    int fdin; // File descriptor for the port
    
    fdin = open(
        port_address, 
          O_RDWR // O_RDWR read and write (CREAD, )
        | O_NOCTTY // port never becomes the controlling terminal 
        | O_NDELAY // use non-blocking I/O
        // CLOCAL don't allow control of the port to be changed
    );

    if (fdin == -1) {
        // Could not open port
        fprintf(stderr, "\r\nSC_openPort: Unable to open port: ", port_address, "%s\n", strerror(errno));
        fprintf(stderr, "\r\nSC_openPort: Unable to open port: ", port_address, "%s\n", strerror(errno));
        return -1;
    }
    
    if ( !isatty(fdin) ) {
        fprintf(stderr, "\r\nSC_openPort: Not a serial device!", port_address, "%s\n", strerror(errno));
        return -1;
    }

    return(fdin);
}

SC_closePort(int fdin)
{
    if (close(fdin) == -1) 
    {
        fprintf(
            stderr, 
                "\r\nSC_closePort: Unable to close the serial device connection!"
        );
        return -1;
    }
    fprintf(stdout, "\r\nSC_closePort: Closed device %s successfully!\r\n",port_address);
    return 1;
}

/**
 * Function to configure interface
 */
void
SC_configureInterface (int fdin)
{ 
    // http://man7.org/linux/man-pages/man3/termios.3.html
    // http://www.unixguide.net/unix/programming/3.6.2.shtml
    struct termios settings;
   
    // get current settings
    int get_settings = -1; 
    if ( get_settings = tcgetattr(fdin, &settings) < 0 ) 
    {
        fprintf(
            stderr, 
            "\r\nSC_configureInterface: failed to get config: %d, %s\n", 
            fdin, 
            strerror(errno)
        );
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nSC_configureInterface: successfully acquired old settings");
    cfmakeraw(&settings); // raw mode: 
    //Input is not assembled into lines and special characters are not processed.

    // attempt to set input and output baud rate to 9600
    if (cfsetispeed(&settings, B9600) < 0 || cfsetospeed(&settings, B9600) < 0) 
    {
        fprintf(stderr, "\r\nSC_configureInterface: failed set BAUD rate: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nSC_configureInterface: successfully set new baud rate");

    // Input flags 
    settings.c_iflag = 0; // disable input processing
    settings.c_iflag &= ~(  
          IGNBRK // disable: ignore BREAK condition on input 
        | BRKINT // convert break to null byte
        | ICRNL  // no CR to NL translation
        | INLCR  // no NL to CR translation
        | PARMRK // don't mark parity errors or breaks
        | INPCK  // no input parity check
        | ISTRIP // don't strip high bit off
        | IXON   // no XON/XOFF software flow control
    ); 
    
    // Output flags
    settings.c_oflag = 0; // disable output processing
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
/*  
    settings.c_lflag &= ~(
          ECHO   // echo off
        | ECHONL // echo newline off
        | ICANON // canonical mode off
        | IEXTEN // extended input processing off
        | ISIG   // signal chars off
    );
*/
    //settings.c_lflag |= (ECHONL | ICANON);

    // Control flags
    settings.c_cflag &= ~(
          PARENB  // no parity bit  
        | CSIZE   // current char size mask
        | CSTOPB  // stop bits: 1 stop bit
        | CRTSCTS // disable hardware flow control
        //| ~PARODD; // even parity
    );
    settings.c_cflag |= (
          B9600    // set BAUD to 9600
        | CS8      // byte size: 8 data bits
        | CREAD   // enable receiver
        //| PARENB  // enable parity bit
        //| PARODD  // odd parity
        //| CLOCAL  // set local mode
    );

    // Read control behaviour 
    settings.c_cc[VMIN]  = 1;     // min bytes to return read
    settings.c_cc[VTIME] = 100;     // read timeout in 1/10 seconds 
    
    // Special input characters
    // only if ICANON is set:
    //   VEOF, VEOL, VERASE, VKILL (and also 
    //   VEOL2, VSTATUS and VWERASE if defined and IEXTEN is set) 
    // only if ICANON is NOT set:
    //   VMIN, VTIME 
    // only if ISIG is set:
    //   VINTR, VQUIT, VSUSP (and also VDSUSP if 
    //   defined and IEXTEN is set) 
    // only if IXON or IXOFF is set:
    //   VSTOP, VSTART 
   
    //tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
    //tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
    fcntl(
        fdin, 
        F_SETFL, // return 0 if no chars available on port 
        FNDELAY
    ); // non-blocking, immediate reads
    
    tcflush( fdin, TCIFLUSH); // flush port before persisting changes 
    
    int apply_settings = -1;
    if ( apply_settings = tcsetattr(fdin, TCSANOW, &settings) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nSC_configureInterface: failed to set config: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);           
    }

    fprintf(stdout, "\r\nSC_configureInterface: successfully applied new settings");
}

// Responses are 8 bytes ack or no ack 
// noop_ack 486520010a0a35a1
// some_ack 486520060a0a3ab0
// tx_ac    486520030a0a37a7
// noack    48652001ffff1f80
int 
SC_write(int fdin, char *bytes, size_t size) 
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

    // Read response
    unsigned char buffer[8]; 
    int chars_read = read(fdin, &buffer, 8);
    //buffer[chars_read] = '\0';

    //usleep(100000);

    if (w>0) {
        return 1;
    } 
}

/** Provide signal handling for SC_read **/
volatile sig_atomic_t stop;
void inthand (int signum) { stop = 1; }

void
SC_read (int fdin) 
{
    // Read response
    unsigned char buffer[1];
    unsigned char response[255];
    int chars_read;    
    int i=0;
    //buffer[chars_read] = '\0';
    
    // Read continuously from serial device
    signal(SIGINT, inthand);
  
    while (!stop)
    {
        // int result = memcmp( bytes, chars_read, 8 );
        if (read(fdin, &buffer, 1) > 0)
        {
            if (buffer[0] != '\r' && buffer[0] != '\n') {
                response[i]=buffer[0];
//                printf("\n i:%d buf:0x%02X msg:0x%02X",i,buffer[0],response[i]);
                i++;
            }
            else {
                // done gathering response!
                response[i] = '\0';
                fprintf(stdout,"\nTotal response hex: %s\n",response);
                SC_interpretResponse(response, i+1);
                buffer[0] = '\0';
                i=0;
                // SC_parseResponse(response);
            }
        }
    }
}

/**
 * struct to hold values of fletcher checksum
 */
typedef struct SC_checksum {
    uint16_t sum1;
    uint16_t sum2;
} SC_checksum;

/** 
 * 16-bit implementation of the Fletcher Checksum
 * returns two 8-bit sums
 * @param data - uint8_t const - data on which to perform checksum
 * @param bytes - size_t - number of bytes to process
 * inspired by http://en.wikipedia.org/wiki/Fletcher%27s_checksum#Optimizations
 */
struct SC_checksum 
SC_fletcher16 (char *data, size_t bytes)
{
    uint16_t sum1 = 0xff, sum2 = 0xff;

    while (bytes)
    {
        size_t tlen = bytes > 20 ? 20 : bytes; 
        bytes -= tlen;
        do
        {
            sum2 += sum1 += *data++;
        }
        while (--tlen);

        // Reduce to 8-bit
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    // final reduction to 8 bits
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);

    // prepare and return checksum values 
    //SC_fletcher_tuple r = { sum2 << 8 , sum1 };
    SC_checksum r;
    r.sum1 = (sum1 & 0xf);
    r.sum2 = sum2;
    return r;
}

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of data stream
 */
unsigned char* 
SC_prepareTransmission(char *payload, size_t length, char *command)
{
    unsigned char *transmission = (char *) malloc(length+10);
    
    // attach sync bytes
    transmission[0] = 0x48;
    transmission[1] = 0x65;

    // attach command bytes
    transmission[2] = command[0];
    transmission[3] = command[1];

    // attach length bytes
    transmission[4] = 0x00;
    transmission[5] = (char) length & 0xff; 

    // generate and attach header checksum
    SC_checksum header_checksum = SC_fletcher16(transmission,10); 
    transmission[6] = (char) header_checksum.sum1 & 0xff;
    transmission[7] = (char) header_checksum.sum2 & 0xff;
    
    // generate and attach payload checksum
    SC_checksum payload_checksum = SC_fletcher16(transmission,length); 
    transmission[8+length] = payload_checksum.sum1;
    transmission[8+length+1] = payload_checksum.sum2; 

    // attach payload and return transmission
    int i;
    for (i=0;i<length;i++)
    {
        transmission[8+i] = payload[i];
        //use strncpy to avoid buffer problems
    }

    return (char*) transmission;
}

/**
 * Function to parse a given frame, verify it, and mine its data
 * @param response - the frame data to be parsed
 * @param length - the entire length of the frame in bytes
 */
unsigned char*
SC_parseResponse (char *response, size_t length) 
{
    unsigned char *data = (char *) malloc(length);

    // prepare container for decoded data
    int data_length = length - 14; // response - header - 4 checksum bytes
    unsigned char *msg = (char *) malloc(data_length);

    int i; int j=0;
    for (i=2;i<7;i++) 
        data[j] = response[i];
    
    // generate and compare header checksum
    SC_checksum h_chksum = SC_fletcher16(data,10); 
    int h_s1_chk = memcmp(&response[8], &h_chksum.sum1, 1);
    int h_s2_chk = memcmp(&response[9], &h_chksum.sum2, 1);
    int h_chk = h_s2_chk + h_s2_chk; // should be zero given valid chk

    // pick up j where it left off
    for (i=8;i<length-2;i++) // read up to, not including, payload chksums
        data[j] = response[i];

    // generate and compare payload checksum
    SC_checksum p_chksum = SC_fletcher16(data,10); 
    int p_s1_chk = memcmp(&response[length-2], &p_chksum.sum1, 1);
    int p_s2_chk = memcmp(&response[length-1], &p_chksum.sum2, 1);
    int p_chk = p_s1_chk + p_s2_chk; // should be zero given valid chk
    
    if (h_chk != 0)
        printf("Invalid header checksum");

    if (p_chk != 0)
        printf("Invalid payload checksum");

    j=0;
    for (i=10;i<data_length;i++)
        msg[j] = response[i];
    
    return (char*) msg; 
}

int
SC_interpretResponse (char *response, size_t length) 
{
    printf("Reponse length: %d\n", length);

    // if response == transmission, He100 device is off!   
    if ((int)response[2]==16) {
        fprintf(stderr,"SC_read: He100 is off!");
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
            default     : value = "Data/Checksum"; break;

        }
        printf(": %s,\n",value);
    }
    return 1;
}

//Hex value of decimal is 2 * 4-bit bytes
char*
SC_dec2Hex(int decimal)
{
    if (decimal > 255) { // only dealing with 1 byte size
        fprintf(
            stderr, 
            "Decimal value must be less than 255, given: %d",
            decimal
        );
        return 0;
    }
    char *hex = (char *) malloc(sizeof(char) * 4);
    int i=0;
    while (decimal) {
        hex[i++] = decimal % 16 + '0';
        decimal /= 16;
    }
    return hex;
}

/* TODO
unsigned char 
SC_hex2Binary(char *src)
{
    unsigned char *out = malloc(strlen(src)/2);
    char buf[3] = {0};

    unsigned char *dst = out;
    while (*src) 
    {
        buf[0] = src[0];
        buf[1] = src[1];
        *dst = strtol(buf, 0, 16);
        dst++; src += 2;
    }
    return out;
}
*/

/** 
 * does no bounds checking, won't work with unicode console input, will crash if passed invalid character 
 **
char
SC_convertHex2Bytes(char hex)
{
    // Create buffer based on input hex string 
    char * buffer = malloc((strlen(hex) / 2 ) + 1);
    char *h = hex; // walk through the buffer 
    char *b = buffer; // point inside buffer 

    // offset into this string is the numeric value 
    char xlate[] = "0123456789abcdef";

    for ( ; *h; h+=2, ++b )
        *b = ((strch(xlate, *h) - xlate) * 16) // Multiply leading digit by 16 
            + ((strchr(xlate, *(h+1))- xlate));
    return b;
}
*/
