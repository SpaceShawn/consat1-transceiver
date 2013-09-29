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

int main(int argc, char** argv) 
{
    unsigned char c='D'; // execution status
    int fd = 0; // serial_device instance
    
    unsigned char bytes[10] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43,0x00,0x00};
    unsigned char hello[15] = {0x48,0x65,0x10,0x03,0x00,0x05,0x18,0x4e,0x68,0x65,0x6c,0x6c,0x6f,0x16,0x2d};

    if (fd = SC_openPort()) 
    { 
        fprintf(stderr, "Successfully opened port");
        SC_configureInterface(fd);
       
        // Write
        int w = write (fd, bytes, 10); // write given 10 bytes to fd 
       
        // Read
        // Responses are 8 bytes ack or no ack 
        // noop_ack 486520010a0a35a1
        // some_ack 486520060a0a3ab0
        // tx_ac    486520030a0a37a7
        // noack    48652001ffff1f80
    	char buffer[8]; 
		int chars_read = read(fd, &buffer, sizeof(buffer));
		//buffer[chars_read] = '\0';
	
        // if response == transmission, He100 device is off!
        // int result = memcmp( bytes, chars_read, 8 );

        // Print    
        printf("\nResponse: \n");
        //printf("%s", buffer, "\n\r");  
        printf("%d", chars_read, "\n\r");  
        printf("Bytesize: ", sizeof(buffer));
    	//printf("%x", buffer); // print as Hexadecimal
	
        // Looping read
        while (c!='q')
        {
            if (read(fd,&c,1)>0)
                write(STDOUT_FILENO,&c,1);
            if (read(STDIN_FILENO,&c,1)>0)
                write(STDOUT_FILENO,"\nSending Transmission:",1);
                write(STDOUT_FILENO,bytes,1);
                write(STDOUT_FILENO,"\nResponse:",1);
                write(fd,bytes,1);
        }
        
        if (close(fd == -1)) 
        {
            fprintf(
                stderr, 
                "close: Unable to close the serial device connection!"
            );
        }                        
        return EXIT_SUCCESS;    
    }
}
