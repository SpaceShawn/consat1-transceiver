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

    if ( fdin = SC_openPort() ) // Input stream
    { 
        // open output file for appending
        FILE *fdout; 
        fdout = fopen("/var/log/space/he100","a");
                 
        fprintf(stderr, "\r\nSuccessfully opened port: %s",port_address);
        SC_configureInterface(fdin);
        
        fprintf(stdout, "\r\nCurrent status of device: %d",fdin);

        // Write noop
        //SC_write(fdin, noop, 10);

        // Write a payload 
        unsigned char *message = "Hello";
        size_t msg_len = 5; // don't forget to change this
        size_t write_len = msg_len + 10; // payload is msg_len + 10 he100 wrapper bytes
        unsigned char command[2] = {CMD_TRANSMIT,CMD_TRANSMIT_DATA}; // {0x10,0x03}

        //unsigned char command[2] = {CMD_TRANSMIT,CMD_RECEIVE_DATA}; // {0x10,0x03}
        //unsigned char message[27] = {0x86, 0xA2, 0x40, 0x40, 0x40, 0x40, 0x60, 0xAC, 0x8A, 0x64, 0x86, 0xAA, 0x82, 0xE1, 0x03, 0xF0, 0x6B, 0x65, 0x6E, 0x77, 0x6F, 0x6F, 0x64, 0x0D, 0x8D, 0x08, 0x63};
        //size_t msg_len = 5; // don't forget to change this

        if ( SC_write(fdin, SC_prepareTransmission(message, msg_len, command), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 

/*       // test SC_fastSetPA()
        int fast_set_pa_level = 9;
        write_len = 1+10; 
        if ( SC_write(fdin, SC_fastSetPA(50), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

/*       // test SC_softReset()
        write_len = 0+10;
        if ( SC_write(fdin, SC_softReset(), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/*       // test SC_setBeaconInterval()
        write_len = 1+10;
        int beacon_interval = 3; // three second interval
        if ( SC_write(fdin, SC_setBeaconInterval(beacon_interval), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/*       // test SC_setBeaconMessage()
        unsigned char* beacon_data = "this is a beacon";
        msg_len = 16;
        write_len = msg_len + 10;
        int beacon_interval = 3; // three second interval
        if ( SC_write(fdin, SC_setBeaconMessage(beacon_interval), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

        // read continuously until SIGINT
        SC_read(fdin);

        // close he100 device
        SC_closePort(fdin);
        
        // close output file
        fclose(fdout);

        return EXIT_SUCCESS;    
    }
}
