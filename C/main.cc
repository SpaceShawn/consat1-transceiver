/*
 * =====================================================================================
 *
 *       Filename:  main.cc
 *
 *    Description:  Test opening the serial device, writing, reading, and closing.
 *
 *        Version:  1.0
 *        Created:  13-09-20 08:30:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Shawn Bulger (), 
 *   Organization:  Space Concordia
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <string.h>
#include "./SC_transceiverLib.h"

int main() 
{
    /* Serial instance and configuration variables */
    int fd = 0; // serial_device instance

    /* Test data for transmission to the HE100 */
    char *hex = "48 65 10 01 00 00 11 43 00 00"; /* HEX for HE100 NOOP command */
    char * bytes[10] = SC_convertHex2Bytes(hex); /* bytearray for transmission */

    if (fd = openPort()) 
    { 
        fprintf(stderr, "Successfully opened port");
        configureInterface(fd);
        int w = write (fd, bytes, 4); // 
        char buf [255];
        int r = read (fd, buf, sizeof buf);
        close(fd);
        return EXIT_SUCCESS;    
    }
}
