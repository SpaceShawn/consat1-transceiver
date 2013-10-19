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
   
    unsigned char noop[10] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43,0x00,0x00};

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
        size_t msg_len = 17; // don't forget to change this
        size_t pay_len = msg_len + 10; // payload is msg_len + 10 he100 wrapper bytes
        unsigned char *message = "Hello from the Q6";
        unsigned char command[2] = {CMD_TRANSMIT,CMD_TRANSMIT_DATA}; // {0x10,0x03}

        if ( SC_write(fdin, SC_prepareTransmission(message, msg_len, command), pay_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
        
        // read continuously until SIGINT
        SC_read(fdin);

        // close he100 device
        SC_closePort(fdin);
        
        // close output file
        fclose(fdout);

        return EXIT_SUCCESS;    
    }
}
