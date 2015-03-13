#ifndef HE100_TTY_SETTINGS_H_
#define HE100_TTY_SETTINGS_H_
/*
 * =====================================================================================
 *
 *       Filename:  SC_he100.h
 *
 *    Description:  Header file for he100 library
 *
 *        Version:  1.0
 *        Created:  13-11-09 01:14:59 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  SHAWN BULGER (),
 *   Organization:
 *
 * =====================================================================================
 */

// TTY devices is stored in inc/XX/he100.h
// XX is set by Makefile

// TODO, define all TTY settings here, most are still set in SC_he100.c

#define HE100_BAUDRATE              B9600    // 9600 bits per second
#define HE100_PARITYBIT             ~PARENB  // no parity bit
#define HE100_BYTESIZE              CS8      // 8 data bits
#define HE100_STOPBITS              ~CSTOPB  // 1 stop bit
#define HE100_HARDWARE_FLOW_CONTROL ~CRTSCTS // disable hardware flow control

#endif
