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
#include <signal.h>
#include "./SC_transceiverLib.h"

int 
main (int argc, char** argv) 
{
    int fdin = 0; // serial_device instance
   
    unsigned char bytes[10] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43,0x00,0x00};
    unsigned char hello[15] = {0x48,0x65,0x10,0x03,0x00,0x05,0x18,0x4e,0x68,0x65,0x6c,0x6c,0x6f,0x16,0x2d};

    if (fdin = SC_openPort()) // Input stream
    { 
        // open output file for appending
        FILE *fdout; 
        fdout = fopen("/var/log/space/he100","a");
                 
        fprintf(stderr, "\r\nSuccessfully opened port: %s",port_address);
        SC_configureInterface(fdin);
        
        fprintf(stdout, "\r\nCurrent status of device: %d",fdin);

        // Write noop
        //SC_write(fdin, bytes);

        // Write Hello
        SC_write(fdin, hello, sizeof(hello));
        char* message = "hello";
        unsigned char command[2] = {CMD_TRANSMIT,CMD_TRANSMIT_DATA}; //{0x10,0x03};
        //SC_write(fdin, SC_prepareTransmission(message, 5, command), 15);
        
        // read continuously until SIGINT
        SC_read(fdin);

        // close he100 device
        SC_closePort(fdin);
        
        // close output file
        fclose(fdout);

        return EXIT_SUCCESS;    
    }
}
