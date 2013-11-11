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

    if ( fdin = HE100_openPort() ) // Input stream
    { 
        // open output file for appending
        FILE *fdout; 
        fdout = fopen("/var/log/space/he100","a");
        
        fprintf(stdout, "\r\nCurrent status of device: %d",fdin);

/* 
        // Write a payload 
        unsigned char *message = "Hello";
        size_t msg_len = 5; // don't forget to change this
        size_t write_len = msg_len+10;
        if ( HE100_write(fdin, HE100_transmitData(message, msg_len), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
*/

/*       // test HE100_NOOP()
        size_t write_len = 0+10; 
        if ( HE100_write(fdin, HE100_NOOP(), write_len) > 0 )
            printf("\r\n NOOP written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

/*       
        // test HE100_fastSetPA()
        int fast_set_pa_level = 9;
        write_len = 1+10; 
        if ( HE100_write(fdin, HE100_fastSetPA(50), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

///*  
        // test HE100_softReset()
        size_t write_len = 0+10;
        if ( HE100_write(fdin, HE100_softReset(), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
//*/

/*       
        // test HE100_setBeaconInterval()
        size_t write_len = 1+10;
        int beacon_interval = 3; // three second interval
        if ( HE100_write(fdin, HE100_setBeaconInterval(beacon_interval), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/*       
        // test HE100_setBeaconMessage()
        unsigned char* beacon_data = "this is a beacon";
        size_t msg_len = 16;
        size_t write_len = msg_len + 10;
        int beacon_interval = 3; // three second interval
        if ( HE100_write(fdin, HE100_setBeaconMessage(beacon_data, msg_len), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/* 
        // Request firmware 
        unsigned char* message = {0};
        size_t msg_len = 0;
        size_t write_len = 10;
        if ( HE100_write(fdin, HE100_readFirmwareRevision(), write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
*/

        // read continuously until SIGINT
        HE100_read(fdin, 10);

        // close he100 device
        HE100_closePort(fdin);
        
        // close output file
        fclose(fdout);

        return EXIT_SUCCESS;    
    }
}
