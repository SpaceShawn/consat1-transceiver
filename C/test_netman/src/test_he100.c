/*
 * =====================================================================================
 *
 *       Filename:  test_he100.c
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
#include <stdio.h>   /*  Standard input/output definitions */
#include <stdint.h>  /*  Standard integer types */
#include <fcntl.h>   /*  File control definitions */
#include <errno.h>   /*  Error number definitions */
#include <string.h>

int 
main (int argc, char** argv) 
{
    int fdin = 0; // serial_device instance

    if ( fdin = SC_openPort() ) // Input stream
    { 
        // open output file for appending
        FILE *fdout; 
        fdout = fopen("/var/log/space/he100","a");
        
        fprintf(stdout, "\r\nCurrent status of device: %d",fdin);

///* 
        // Write a payload 
        unsigned char *message = "Hello";
        size_t msg_len = 5; // don't forget to change this
        size_t write_len = msg_len+10;
        if ( SC_write(fdin, SC_transmitData(message, msg_len), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
//*/

/*       // test SC_NOOP()
        size_t write_len = 0+10; 
        if ( SC_write(fdin, SC_NOOP(), write_len) > 0 )
            printf("\r\n NOOP written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

/*       
        // test SC_fastSetPA()
        int fast_set_pa_level = 9;
        write_len = 1+10; 
        if ( SC_write(fdin, SC_fastSetPA(50), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

/*  
        // test SC_softReset()
        size_t write_len = 0+10;
        if ( SC_write(fdin, SC_softReset(), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/*       
        // test SC_setBeaconInterval()
        size_t write_len = 1+10;
        int beacon_interval = 3; // three second interval
        if ( SC_write(fdin, SC_setBeaconInterval(beacon_interval), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/*       
        // test SC_setBeaconMessage()
        unsigned char* beacon_data = "this is a beacon";
        size_t msg_len = 16;
        size_t write_len = msg_len + 10;
        int beacon_interval = 3; // three second interval
        if ( SC_write(fdin, SC_setBeaconMessage(beacon_data, msg_len), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/* 
        // Request firmware 
        unsigned char* message = {0};
        size_t msg_len = 0;
        size_t write_len = 10;
        if ( SC_write(fdin, SC_readFirmwareRevision(), write_len) > 0 )
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
