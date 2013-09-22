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

char *port_address = "/dev/ttyS0";

int 
open_port(void)
{
    int fd; // File descriptor for the port
    fd = open(port_address, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
        // Could not open port
        fprintf(stderr, "open_port: Unable to open", port_address, "%s\n", strerror(errno));
        return false;
    }

    return(fd);
}

void
configure_interface (int fd)
{
    struct termios settings;

    if ( (rv = tcgetattr(fd, &settings)) < 0 ) 
    {
        fprintf(stderr, "Failed to get attribute: %d, %s\n", fd, strerror(errno));
        exit(EXIT_FAILURE);
    }

    tcgetattr(fd, &settings); // get current settings
    cfsetispeed(&settings, B9600); // set input BAUD rate to 9600
    cfsetospeed(&settings, B9600); // set output BAUD rate to 9600
    cfmakeraw(&settings);

    settings.c_cflag |= (CLOCAL | CREAD) // enable receiver and set local mode
    settings.c_cflag |= ~PARENB;  // no parity bit 
    settings.c_cflag &= ~CSTOPB;  // 1 stop bit
    settings.c_cflag &= ~CSIZE;   // mask data size (needed??)
    settings.c_cflag |= CS8;      // 8 data bits
    settings.c_cflag &= ~CRTSCTS; // disable hardware flow control
    settings.c_cc[VMIN]  = 1;     // 
    settings.c_cc[VTIME] = 2;     // 

    /* Set the preceeding attributes */
    if ( (rc = tcsetattr(fd, TCSANOW, &settings)) < 0 ) 
    {
        fprintf(stderr, "Failed to get attribute: %d, %s\n", fd, strerror(errno));
        exit(EXIT_FAILURE);           
    }

    return fd;
}
