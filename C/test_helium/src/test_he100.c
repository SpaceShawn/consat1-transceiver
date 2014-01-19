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
#include <stdio.h>      /*  Standard input/output definitions */
#include <stdint.h>     /*  Standard integer types */
#include <string.h>     /*  String function definitions */
#include <unistd.h>     /*  UNIX standard function definitions */
#include <signal.h>     /*  Signal handling */
#include <fcntl.h>      /*  File control definitions */
#include <errno.h>      /*  Error number definitions */
#include <termios.h>    /*  POSIX terminal control definitions */
#include <string.h>
#include "NamedPipe.h"  /*  Named pipe header */
#include "SC_he100.h"   /*  Helium 100 header file */
#include "timer.h"
/** 
 * uses fgets() to read up to MAX_LINE - 1 characters into the buffer 'in'. It strips preceding whitespace and returns a pointer to the first non-whitespace character
 * https://stackoverflow.com/questions/2600528/c-readline-function
 */
/* 
char *Readline(char *in) {
    char *cptr;

    if (cptr = fgets(in, MAX_LINE, stdin)) {
        //  kill preceding whitespace but leave \n so we're guaranteed to have something
        while(*cptr == ' ' || *cptr == '\t') {
            cptr++;
        }
        return cptr;    
    } else {
        return 0;
    }
}
*/
/** Provide signal handling **/
volatile sig_atomic_t stop;
void inthand (int signum) { stop = 1; }

int 
main (int argc, char** argv) 
{
    signal(SIGINT, inthand);
    int fdin = 0; // serial_device instance

    if ( fdin = HE100_openPort() ) // Input stream
    { 
        fprintf(stdout, "\r\nCurrent status of device: %d",fdin);

/*      // test the timer
        timer_t test_timer = timer_get();
	    timer_start(&test_timer, 2);

        while (!timer_complete(&test_timer))
        {
            fprintf(stdout,"\r\nwaiting...");
            sleep(1);
        }
*/

/*  
        // test HE100_softReset()
        if ( HE100_softReset(fdin) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

///* 
        // Write a payload 
        unsigned char message[27] = "I can't let you do that Ty";
        size_t msg_len = 26; // don't forget to change this
        size_t write_len = msg_len+10;            
        printf("\r\n Transmitting test message!");
        if ( HE100_transmitData(fdin, message, msg_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");

        printf("\r\n Beginning spamming! test message!");
        while (!stop) {
            // sleep(0.1);
            if ( HE100_transmitSpam(fdin, message, msg_len) > 0 )
                printf("\r\n Message written successfully!");
            else  
                printf("\r\n Problems writing to serial device"); 
        }
//*/

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
        if ( HE100_fastSetPA(fdin, fast_set_pa_level) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");       
*/

/*       
        // test HE100_setBeaconInterval()
        int beacon_interval = 4; // three second interval
        if ( HE100_setBeaconInterval(fdin,beacon_interval) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/*       
        // test HE100_setBeaconMessage()
        unsigned char* beacon_data = "this is a beacon";
        size_t bcn_msg_len = 16;
        if ( HE100_setBeaconMessage(fdin, beacon_data, bcn_msg_len) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device");
*/

/* 
        // Request firmware 
        if ( HE100_readFirmwareRevision(fdin) > 0 )
            printf("\r\n Message written successfully!");
        else  
            printf("\r\n Problems writing to serial device"); 
*/

///*      // read continuously until SIGINT
        HE100_read(fdin, 15);
//*/
        // close he100 device
        HE100_closePort(fdin);
         
        return EXIT_SUCCESS;    
    }
}
