/*
 * =====================================================================================
 *
 *       Filename:  SC_transceiverLib.cc
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
#include <string.h>  /*  String function definitions */
#include <unistd.h>  /*  UNIX standard function definitions */
#include <fcntl.h>   /*  File control definitions */
#include <errno.h>   /*  Error number definitions */
#include <termios.h> /*  POSIX terminal control definitions */
//#include "./lib/settings.c" 

const char *port_address = "/dev/ttyS0";

int
SC_openPort(void)
{
    int fd; // File descriptor for the port
    
    fd = open(
            port_address, 
            // O_RDWR read and write (CREAD)
            // O_NCTTY prevent other input from affecting what is read 
            // CLOCAL don't allow control of the port to be changed
            // O_NDELAY don't care if other side is connected
            O_RDWR | O_NOCTTY | O_NDELAY
        );

    if (fd == -1) {
        // Could not open port
        fprintf(stderr, "open_port: Unable to open", port_address, "%s\n", strerror(errno));
        return -1;
    }

    return(fd);
}

void
SC_configureInterface (int fd)
{
    int rv = 0; // return value
    struct termios settings;
    
    if ( (rv = tcgetattr(fd, &settings)) < 0 ) 
    {
        fprintf(stderr, "Failed to get attribute: %d, %s\n", fd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    tcgetattr(fd, &settings); // get current settings
    cfsetispeed(&settings, B9600); // set input BAUD rate to 9600
    cfsetospeed(&settings, B9600); // set output BAUD rate to 9600
    cfmakeraw(&settings); // make raw
    
    settings.c_iflag = 0;
    settings.c_oflag = 0; // raw output, ignore line breaks

    // Input: canonical/non-canonical/asynchronous/multiple-sources
    //    settings.c_lflag = 0; // no signaling chars, no echo, no canonical processing
    settings.c_lflag = ICANON; // canonical (terminal) mode
    
    settings.c_cflag |= (CLOCAL | CREAD); // enable receiver and set local mode
    settings.c_cflag |= ~PARENB;  // no parity bit 
    //settings.c_cflag |= PARENB;  // enable parity bit
    //settings.c_cflag |= PARODD;  // odd parity
    //settings.c_cflag |= ~PARODD; // even parity

    settings.c_cflag &= ~CSIZE;   // mask data size, clear current data size

    settings.c_cflag |= B9600;    // set BAUD to 9600
    settings.c_cflag |= CS8;      // byte size: 8 data bits
    settings.c_cflag &= ~CSTOPB;  // stop bits: 1 stop bit
    settings.c_cflag &= ~CRTSCTS; // disable hardware flow control
    settings.c_cc[VMIN]  = 1;     // read doesn't block
    settings.c_cc[VTIME] = 5;     // read timeout in 1/10 seconds 
    fcntl(STDIN_FILENO, F_SETFL); // non-blocking reads
   
    tcflush( fd, TCIFLUSH); // flush port 
    if ( (rv = tcsetattr(fd, TCSANOW, &settings)) < 0 ) // apply attributes
    {
        fprintf(stderr, "Failed to get attribute: %d, %s\n", fd, strerror(errno));
        exit(EXIT_FAILURE);           
    }
}

/* 
unsigned char *SC_hex2Binary(char *src)
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
