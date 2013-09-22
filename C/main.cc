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
#include <SC_transceiverLib.cc>

int main() 
{
    /* Serial instance and configuration variables */
    int serial_device = 0; // serial_device instance
    int rv = 0; // return value

    /* Test data for transmission to the HE100 */
    string hex = "48651001000011430000"; // HEX for HE100 NOOP command 
    unsigned char bytes[10] = null;      // bytearray for transmission
    
    if (fd = open_port()) 
    {    
        if (configure_interface(fd)) {
            int w = write (fd, bytes , 4); // 
            char buf [255];
            int r = read (fd, buf, sizeof buf);
            close(fd);
            return EXIT_SUCCESS;
        }
    }
}
