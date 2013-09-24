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
#include <string.h>
#include "./SC_transceiverLib.h"

int main() 
{
    int fd = 0; // serial_device instance
    char *hex = "48 65 10 01 00 00 11 43 00 00"; /* HEX for HE100 NOOP command */
    unsigned char bytes[10] = {0x48, 0x65, 0x10, 0x01, 0x00, 0x00, 0x11, 0x43, 0x00, 0x00};

    if (fd = openPort()) 
    { 
        fprintf(stderr, "Successfully opened port");
        configureInterface(fd);
        int w = write (fd, bytes, 4); // 
        char buf [255];
		while(1)
		{
			char buffer[255];
			int chars_read = read(fd, &buffer, sizeof(buffer));
			buffer[chars_read] = '\0';
			printf("%s", buffer);
		}
        close(fd);
        return EXIT_SUCCESS;    
    }
}
