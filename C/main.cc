/*
 * =====================================================================================
 *
 *       Filename:  main.cc
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
#include <SC_transceiverLib.cc>

int main() {
    int transceiver;
    string hex = "48651001000011430000"; // NOOP
    unsigned char bytes[10] = null;

    if (transceiver = open_port()) {
        n = write(fd, bytes , 4);
    }
}
