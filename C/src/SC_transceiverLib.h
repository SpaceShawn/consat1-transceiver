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

//const char *port_address = "/dev/ttyS2"; // lt-stone

int
SC_openPort(void)
{
    int fd; // File descriptor for the port
    
    fd = open(
        port_address, 
          O_RDWR // O_RDWR read and write (CREAD, )
        | O_NOCTTY // port never becomes the controlling terminal 
        | O_NDELAY // use non-blocking I/O
        // CLOCAL don't allow control of the port to be changed
    );

    if (fd == -1) {
        // Could not open port
        fprintf(stderr, "\r\nSC_openPort: Unable to open port: ", port_address, "%s\n", strerror(errno));
        return -1;
    }
    
    if ( !isatty(fd) ) {
        fprintf(stderr, "\r\nSC_openPort: Not a serial device!", port_address, "%s\n", strerror(errno));
        return -1;
    }

    return(fd);
}

// Responses are 8 bytes ack or no ack 
// noop_ack 486520010a0a35a1
// some_ack 486520060a0a3ab0
// tx_ac    486520030a0a37a7
// noack    48652001ffff1f80
int 
SC_write(int fd, char *bytes) {
    // Write noop
    fprintf(stdout, "\r\nWriting to device: %s\r", bytes);
    int w = write (fd, bytes, 10); // write given 10 bytes to fd 
    fprintf(stdout, "\r\nWrite size: %d",w);

    // Read response
    unsigned char buffer[8]; 
    int chars_read = read(fd, &buffer, sizeof(buffer));
    //buffer[chars_read] = '\0';

    // if response == transmission, He100 device is off!
    // int result = memcmp( bytes, chars_read, 8 );

    // Print    
    fprintf(stdout, "\nResponse:  %s", buffer);
    //printf("%hhX", buffer, "\n\r");

    fprintf(stdout, "\r\nResponse Size: %d", sizeof(buffer));
    int i=0;

    printf("\r\nMessage:\n");
    //fprintf(stdout, "%d", chars_read, "\n\r");  

    for (i=0; i<chars_read; i++) 
    {
        printf("%02X ",buffer[i]);
    }

    if (sizeof(buffer)>0) {
        return 1;
    } 
}

void
SC_read(int fd) {
    // Read until receiving the 'q' input character from user!
    while (c!='q')
    {
        char buffer[255];
        response = read(fd,buffer,255);
        buffer[response]=0;
        printf(":%s:%d\n", buffer, response);       
    }
/*
         // Looping read
        fprintf(stdout, "\r\nStarting looping read...");
        while (c!='q')
        {
            if (read(fd,&c,1)>0)
                write(STDOUT_FILENO,&c,1);
            if (read(STDIN_FILENO,&c,1)>0)
                write(STDOUT_FILENO,"\nSending Transmission:",1);
                write(STDOUT_FILENO,bytes,1);
                write(STDOUT_FILENO,"\nResponse:",1);
                write(fd,bytes,1);
        }
*/
}

void
SC_configureInterface (int fd)
{ 
    // http://man7.org/linux/man-pages/man3/termios.3.html
    // http://www.unixguide.net/unix/programming/3.6.2.shtml
    struct termios settings;
   
    // get current settings
    int get_settings = -1; 
    if ( get_settings = tcgetattr(fd, &settings) < 0 ) 
    {
        fprintf(
            stderr, 
            "\r\nSC_configureInterface: failed to get config: %d, %s\n", 
            fd, 
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
        fprintf(stderr, "\r\nSC_configureInterface: failed set BAUD rate: %d, %s\n", fd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nSC_configureInterface: successfully set new baud rate");

    // Input flags 
    settings.c_iflag = 0; // disable input processing
    settings.c_iflag &= ~(  
          IGNBRK // disable -> ignore BREAK condition on input 
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
    // settings.c_lflag = 0; // disable line processing 
    settings.c_lflag &= ~(
          ECHO   // echo off
        | ECHONL // echo newline off
        | ICANON // canonical mode off
        | IEXTEN // extended input processing off
        | ISIG   // signal chars off
    );
    
    // Character processing flags
    settings.c_cflag &= ~(
          PARENB  // no parity bit  
        | CSIZE   // current char size mask
        | CSTOPB  // stop bits: 1 stop bit
        | CRTSCTS // disable hardware flow control
        //| ~PARODD; // even parity
    );
    settings.c_cflag &= (
          B9600    // set BAUD to 9600
        | CS8      // byte size: 8 data bits
        | CREAD   // enable receiver
        //| PARENB  // enable parity bit
        //| PARODD  // odd parity
        //| CLOCAL  // set local mode
    );

    // Read control behaviour 
    settings.c_cc[VMIN]  = 1;     // 1 byte is enough to return read
    settings.c_cc[VTIME] = 30;     // read timeout in 1/10 seconds 
    
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
    fcntl(STDIN_FILENO, F_SETFL); // non-blocking reads
    
    tcflush( fd, TCIFLUSH); // flush port before persisting changes 
    int apply_settings = -1;
    if ( apply_settings = tcsetattr(fd, TCSANOW, &settings) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nSC_configureInterface: failed to set config: %d, %s\n", fd, strerror(errno));
        exit(EXIT_FAILURE);           
    }
    fprintf(stdout, "\r\nSC_configureInterface: successfully applied new settings");
}

/**
 * struct to hold values of fletcher checksum
 */
struct Tuple 
{
    uint16_t sum1;
    uint16_t sum2;
};

/** 
 * 8-bit implementation of the Fletcher Checksum
 * @param data - uint8_t const - data on which to perform checksum
 * @param bytes - size_t - number of bytes to process
 * inspired by http://en.wikipedia.org/wiki/Fletcher%27s_checksum#Optimizations
 */
struct Tuple
SC_fletcher16( char *data, size_t bytes)
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
    Tuple r = { sum2, sum1 };
    return r;
}

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of data stream
 */
unsigned char 
SC_prepareTransmission(char payload, size_t length)
{
    //!- payload should be in bytes
    
    
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
