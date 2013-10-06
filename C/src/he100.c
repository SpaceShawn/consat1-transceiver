/*
 * =====================================================================================
 *
 *       Filename:  he100.c
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

int main(int argc, char** argv) 
{
    int fd = 0; // serial_device instance
   
    unsigned char bytes[10] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43,0x00,0x00};
    unsigned char hello[15] = {0x48,0x65,0x10,0x03,0x00,0x05,0x18,0x4e,0x68,0x65,0x6c,0x6c,0x6f,0x16,0x2d};

    if (fd = SC_openPort()) 
    { 
        fprintf(stderr, "\r\nSuccessfully opened port: %s",port_address);
        SC_configureInterface(fd);
        
        fprintf(stdout, "\r\nCurrent status of device: %d",fd);

        // Write noop
        //SC_write(fd, bytes);

        // Write Hello
        SC_write(fd, hello, sizeof(hello));

        // Read continuously from serial device
        SC_read(fd);
        
        if (close(fd == -1)) 
        {
            fprintf(
                stderr, 
                "close: Unable to close the serial device connection!"
            );
        }
        fprintf(stdout, "Closed device %s successfully!\r\n",port_address);
        return EXIT_SUCCESS;    
    }
}
