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
#include "SC_he100.h"
#include "timer.h"

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

///*      // test the timer
        timer_t test_timer = timer_get();
	    timer_start(&test_timer, 2);

        while (!timer_complete(&test_timer))
        {
            fprintf(stdout,"\r\nwaiting...");
            sleep(1);
        }
//*/

/* 
        // Write a payload 
        unsigned char *message = "LOLOL";
        size_t msg_len = 5; // don't forget to change this
        size_t write_len = msg_len+10;
        //if ( HE100_write(fdin, HE100_transmitData(message, msg_len), write_len) > 0 )
        if ( HE100_transmitData(fdin, message, msg_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
*/

/* 
        // send bogus (custom) bytes to get a NOACK 
        unsigned char bogus[8] = {0x48,0x65,0x10,0x7e,0x00,0x00,0x65,0x65};
        size_t write_len = 8;
        if ( HE100_write(fdin, bogus, write_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
*/

/*       // test HE100_NOOP()
        if ( HE100_NOOP(fdin) > 0 )
            printf("\r\n NOOP written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

/*       
        // test HE100_fastSetPA()
        int fast_set_pa_level = 9;
        write_len = 1+10; 
        if ( HE100_fastSetPA(fdin, fast_set_pa_level) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

/*  
        // test HE100_softReset()
        if ( HE100_softReset(fdin) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

///*       
        // test HE100_setBeaconInterval()
        size_t write_len = 1+10;
        int beacon_interval = 3; // three second interval
        if ( HE100_setBeaconInterval(fdin,beacon_interval) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
//*/

/*       
        // test HE100_setBeaconMessage()
        unsigned char* beacon_data = "this is a beacon";
        size_t msg_len = 16;
        int beacon_interval = 3; // three second interval
        if ( HE100_setBeaconMessage(fdin, beacon_data, msg_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/* 
        // Request firmware 
        if ( HE100_readFirmwareRevision(fdin)) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
*/

///*      // read continuously until SIGINT
        HE100_read(fdin, 10);
//*/
        // close he100 device
        HE100_closePort(fdin);
        
        // close output file
        fclose(fdout);

        return EXIT_SUCCESS;    
    }
}
