/* 
 * =====================================================================================
 *
 *       Filename:  timer.c
 *
 *    Description:  Library to allow unlimited generic timers, compile with -lrt 
 *
 *        Version:  1.0
 *        Created:  13-11-06 14:23:35 PM
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
#include <unistd.h>  /*  UNIX standard function definitions */
#include <errno.h>   /*  Error number definitions */
#include <signal.h>
#include <time.h>

/* //usr/include/time.h
struct timespec {
    __time_t tv_sec; // seconds 
    long int tv_nsec; // nano seconds 
};
*/

typedef struct SC_timer {
    struct timespec start_time;
    struct timespec end_time;
} SC_timer;

struct SC_timer 
SC_setTimer (time_t timer_duration)
{
    struct timespec start, stop;
    struct SC_timer timer;

    if ( clock_gettime(CLOCK_MONOTONIC, &start) == 0 && clock_gettime(CLOCK_MONOTONIC, &stop) == 0 ) 
    {
        timer.start_time = start;
        timer.end_time = stop;
        return timer;
    } 
    else 
    {
        perror( "\r\n Failed setting timer" );
        exit(EXIT_FAILURE); 
    }
}

time_t 
SC_timerRemaining (struct SC_timer ap_timer) 
{
    struct timespec current;

    if( clock_gettime(CLOCK_MONOTONIC, &current) == 0 )
        return (ap_timer.end_time.tv_sec - current.tv_sec); 
}        

// test a manual timer based on CLOCK_MONOTONIC
time_t test_timer_duration = 5000; /* ms */
struct SC_timer test_timer;

test_timer = SC_setTimer(test_timer_duration);

while ( SC_timerRemaining(test_timer) > 0 )
    fprintf(stdout, "\r\nTime remaining: %d", (int)test_time_remaining);
