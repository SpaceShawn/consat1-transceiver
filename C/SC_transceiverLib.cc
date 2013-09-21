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

int 
open_port(void)
{
    int fd; // File descriptor for the port
    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1) {
        // Could not open port
        fprintf(stderr, "open_port: Unable to open /dev/ttyS0 - %s\n", strerror(errno));
    }

    return(fd);
}
