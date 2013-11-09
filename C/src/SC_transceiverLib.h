/* RENAME TO SC_transeiverLib.c, .h files should not contain actual code or function bodies
 * =====================================================================================
 *
 *       Filename:  SC_transceiverLib.c
 *
 *    Description:  Library to expose He100 functionality. Build as static library. 
 *
 *        Version:  1.0
 *        Created:  13-09-20 08:23:35 PM
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
#include <string.h>  /*  String function definitions */
#include <unistd.h>  /*  UNIX standard function definitions */
#include <fcntl.h>   /*  File control definitions */
#include <errno.h>   /*  Error number definitions */
#include <termios.h> /*  POSIX terminal control definitions */
#include "he100.h"   /*  Header file that exposes the correct serial device location */
#include "./he100_cfg.h" /* Header file that stores the He100 config and command structure */

int
SC_openPort(void)
{
    int fdin; // File descriptor for the port
    
    fdin = open(
        port_address, 
          O_RDWR // O_RDWR read and write (CREAD, )
        | O_NOCTTY // port never becomes the controlling terminal 
        | O_NDELAY // use non-blocking I/O
        | O_NONBLOCK
        // CLOCAL don't allow control of the port to be changed
    );

    if (fdin == -1) {
        // Could not open port
        fprintf(stderr, "\r\nSC_openPort: Unable to open port: ", port_address, "%s\n", strerror(errno));
        return -1;
    }
    
    if ( !isatty(fdin) ) {
        fprintf(stderr, "\r\nSC_openPort: Not a serial device!", port_address, "%s\n", strerror(errno));
        return -1;
    }

    return(fdin);
}

SC_closePort(int fdin)
{
    if (close(fdin) == -1) 
    {
        fprintf(
            stderr, 
                "\r\nSC_closePort: Unable to close the serial device connection!"
        );
        return -1;
    }
    fprintf(stdout, "\r\nSC_closePort: Closed device %s successfully!\r\n",port_address);
    return 1;
}

/**
 * Function to configure interface
 * @param fdin - the file descriptor representing the serial device
 * REF: http://man7.org/linux/man-pages/man3/termios.3.html
 * REF: http://www.unixguide.net/unix/programming/3.6.2.shtml
 */
void
SC_configureInterface (int fdin)
{ 
    struct termios settings;
   
    // get current settings
    int get_settings = -1; 
    if ( get_settings = tcgetattr(fdin, &settings) < 0 ) 
    {
        fprintf(
            stderr, 
            "\r\nSC_configureInterface: failed to get config: %d, %s\n", 
            fdin, 
            strerror(errno)
        );
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nSC_configureInterface: successfully acquired old settings");

    // attempt to set input and output baud rate to 9600
    if (cfsetispeed(&settings, B9600) < 0 || cfsetospeed(&settings, B9600) < 0) 
    {
        fprintf(stderr, "\r\nSC_configureInterface: failed set BAUD rate: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "\r\nSC_configureInterface: successfully set new baud rate");

    // Input flags 
    //settings.c_iflag = 0; // disable input processing
    settings.c_iflag &= ~(  
          IGNBRK // disable: ignore BREAK condition on input 
        | BRKINT // convert break to null byte
        | ICRNL  // no CR to NL translation
        | INLCR  // no NL to CR translation
        | PARMRK // don't mark parity errors or breaks
        | INPCK  // no input parity check
        | ISTRIP // don't strip high bit off
    //    | IXON   // no XON/XOFF software flow control
    ); 
    settings.c_iflag |= (
          IXON
        | IXOFF
        | IGNPAR // ignore bytes with parity errors
        | ICRNL  // map CR to NL (otherwise CR input on other computer will not terminate input)
    //    | INLCR  // map NL to CR (otherwise CR input on other computer will not terminate input)
        );
    
    // Output flags
    settings.c_oflag = 0; // disable output processing, raw output
/*    
    settings.c_oflag &= ~(
          OCRNL  // turn off output processing
        | ONLCR  // no CR to NL translation 
        | ONLRET // no NL to CR-NL translation
        | ONOCR  // no NL to CR translation
        | ONOEOT // no column 0 CR suppression
        | OFILL  // no Ctrl-D suppression, no fill characters,
        | OLCUC  // no case mapping
        | OPOST  // no local output processing
    );
*/

    // Line processing flags
    settings.c_lflag = 0; // disable line processing 
    
    // settings.c_lflag = ICANON; // enable canonical input
    //cfmakeraw(&settings); // raw mode Input is not assembled into lines and special characters are not processed.
    
    settings.c_lflag = ECHONL; 
/*  
    settings.c_lflag &= ~(
          ECHO   // echo off
        | ECHONL // echo newline off
        | ICANON // canonical mode off
        | IEXTEN // extended input processing off
        | ISIG   // signal chars off
    );
*/

    // Control flags
    settings.c_cflag &= ~( // disable stuff
          PARENB  // no parity bit  
        | CSIZE   // mask the character size bits 
        | CSTOPB  // stop bits: 1 stop bit
        | CRTSCTS // disable hardware flow control
        //| ~PARODD; // even parity
    );
    settings.c_cflag |= ( // enable stuff
          B9600    // set BAUD to 9600
        | CS8      // byte size: 8 data bits
        | CREAD   // enable receiver
        //| PARENB  // enable parity bit
        //| PARODD  // odd parity
        //| CLOCAL  // set local mode
    );

    // ONLY non-canonical read control behaviour 
    settings.c_cc[VMIN]  = 1;     // min bytes to return read
    settings.c_cc[VTIME] = 30;    // read timeout in 1/10 seconds 

    //   VEOF, VEOL, VERASE, VKILL (and also 
    //   VEOL2, VSTATUS and VWERASE if defined and IEXTEN is set) 
    
/* 
    // ONLY canonical mode parameters
    settings.c_cc[VINTR]    = 0;     //  Ctrl-c  
    settings.c_cc[VQUIT]    = 0;     //  Ctrl-\
    settings.c_cc[VERASE]   = 0;     //  del 
    settings.c_cc[VKILL]    = 0;     //  @ 
    settings.c_cc[VEOF]     = 4;     //  Ctrl-d 
    settings.c_cc[VTIME]    = 0;     //  inter-character timer unused 
    settings.c_cc[VMIN]     = 1;     //  blocking read until 1 character arrives 
    settings.c_cc[VSWTC]    = 0;     //  '\0' 
    settings.c_cc[VSTART]   = 0;     //  Ctrl-q 
    settings.c_cc[VSTOP]    = 0;     //  Ctrl-s 
    settings.c_cc[VSUSP]    = 0;     //  Ctrl-z 
    settings.c_cc[VREPRINT] = 0;     //  Ctrl-r 
    settings.c_cc[VDISCARD] = 0;     //  Ctrl-u 
    settings.c_cc[VWERASE]  = 0;     //  Ctrl-w 
    settings.c_cc[VLNEXT]   = 0;     //  Ctrl-v 
    settings.c_cc[VEOL]     = 0;     //  end-of-line '\0' 
    settings.c_cc[VEOL2]    = 0;     //  end-of-line '\0' 
*/

    // only if ISIG is set:
    //   VINTR, VQUIT, VSUSP (and also VDSUSP if 
    //   defined and IEXTEN is set) 
    
    // only if IXON or IXOFF is set:
    //   VSTOP, VSTART 
   
    //tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
    //tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
    fcntl(
        fdin, 
        F_SETFL,  
        FNDELAY // return 0 if no chars available on port (non-blocking)
    ); // immediate reads
    
    tcflush(fdin, TCIFLUSH); // flush port before persisting changes 
    
    int apply_settings = -1;
    if ( apply_settings = tcsetattr(fdin, TCSANOW, &settings) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nSC_configureInterface: failed to set config: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);           
    }
/*   
    int flush_device = -1;
    if ( flush_device = tcsetattr(fdin, TCSAFLUSH, &settings) < 0 ) // apply attributes
    {
        fprintf(stderr, "\r\nSC_configureInterface: failed to flush device: %d, %s\n", fdin, strerror(errno));
        exit(EXIT_FAILURE);           
    }
*/
    fprintf(stdout, "\r\nSC_configureInterface: successfully applied new settings and flush device");
}

/** 
 * Function to write a given byte sequence to the serial device
 * @param fdin - the file descriptor representing the serial device
 * @param bytes - the char array containing the byte sequence to write
 * @param size - the length of the array in bytes
 */
int 
SC_write(int fdin, unsigned char *bytes, size_t size) 
{
    // Output outgoing transmission
    fprintf(stdout, "\r\nWriting to device: ");

    // Write byte array
    int w = write (fdin, bytes, size); // write given 10 bytes to fdin 
    int j=0;
    for (j=0; j<size; j++) 
    {
        printf("%02X ",bytes[j]);
    }    
    fprintf(stdout, "\r\nWrite size: %d",w);

    //fflush(fdin);

    if (w>0) {
        return 1;
    } 
}

/**
 * struct to hold values of fletcher checksum
 */
typedef struct SC_checksum {
    uint8_t sum1;
    uint8_t sum2;
} SC_checksum;

/** 
 * 16-bit implementation of the Fletcher Checksum
 * returns two 8-bit sums
 * @param data - uint8_t const - data on which to perform checksum
 * @param bytes - size_t - number of bytes to process
 * inspired by http://en.wikipedia.org/wiki/Fletcher%27s_checksum#Optimizations
 */
struct SC_checksum 
SC_fletcher16unef (char *data, size_t bytes)
{
    uint8_t sum1=0, sum2=0;
    int i=0;

    for (i=0; i<bytes; i++) {
        // calculate sums
        sum1 = (sum1 + (data[i] % 255));
        sum2 += sum1;
        // Reduce to 8-bit
        //printf ("\r\ni:%d d:%d 1:%d 2:%d ",i,(char)data[i],sum1,sum2);
    }

    // prepare and return checksum values 
    SC_checksum r;
    r.sum1 = (sum1 % 255);
    r.sum2 = (sum2 % 255);
    return r;
}

/**
 * Optimized Fletcher Checksum
 */
struct SC_checksum
SC_fletcher16 (char *data, size_t bytes)
{
    //uint8_t sum1 = 0xff, sum2 = 0xff;
    uint8_t sum1 = 0, sum2 = 0;

    while (bytes)
    {
        size_t tlen = bytes > 20 ? 20 : bytes; 
        bytes -= tlen;
        do
        {
            sum2 += sum1 += *data++;
            //printf ("\r\ni:%d d:%d 1:%d 2:%d ",bytes,*data,sum1,sum2);
        }
        while (--tlen);

        // Reduce to 8-bit
        //sum1 = (sum1 & 0xff) + (sum1 >> 8);
        //sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    // prepare and return checksum values 
    SC_checksum r;
    // final reduction to 8 bits
    //r.sum1 = (sum1 & 0xff);
    r.sum1 = sum1;
    //r.sum2 = (sum2 & 0xff);
    r.sum2 = sum2;
    
    return r;
}

/**
 * Function to parse a given frame, validate it, and return its data
 * @param response - the frame data to be validated 
 * @param length - the entire length of the frame in bytes
 */
//unsigned char*
int
SC_validateResponse (char *response, size_t length) 
{
    fprintf(stdout,"\r\n  SC_validateResponse: validating %d byte message",length);
    unsigned char *data = (char *) malloc(length);
    int r=1;

    // prepare container for decoded data
    int data_length = length - 14; // response - header - 4 checksum bytes
    unsigned char *msg = (char *) malloc(data_length);

    int i; int j=0;
    for (i=2;i<7;i++) 
        data[j] = response[i];
    
    // generate and compare header checksum
    SC_checksum h_chksum = SC_fletcher16(data,10); 
    int h_s1_chk = memcmp(&response[8], &h_chksum.sum1, 1);
    int h_s2_chk = memcmp(&response[9], &h_chksum.sum2, 1);
    int h_chk = h_s2_chk + h_s2_chk; // should be zero given valid chk

    // pick up j where it left off
    for (i=8;i<length-2;i++) // read up to, not including, payload chksums
        data[j] = response[i];

    // generate and compare payload checksum
    //SC_checksum p_chksum = SC_fletcher16(data,10); 
    SC_checksum p_chksum = SC_fletcher16(data,length-2); // chksum everything except 'He'
    int p_s1_chk = memcmp(&response[length-2], &p_chksum.sum1, 1);
    int p_s2_chk = memcmp(&response[length-1], &p_chksum.sum2, 1);
    int p_chk = p_s1_chk + p_s2_chk; // should be zero given valid chk
    
    if (response[4] == response[5] ) 
    {
        if (response[4] == 10)
            fprintf(stdout,"\r\n  HE100: Acknowledge");
        else if (response[4] = 255)
            printf("\r\n  HE100: No-Acknowledge");
        else    
            printf("\r\n  HE100: Something strange!");
    } 
    else 
    {
        if (h_chk != 0) {
            fprintf(stdout,"\r\nInvalid header checksum");
            r=-1;
        }
        if (p_chk != 0) {
            fprintf(stdout,"\r\nInvalid payload checksum");
            r=-1;
        }
        
        printf("\r\nMessage: ");
        //j=0;
        for (i=10;i<data_length;i++)
            fprintf(stdout,"%s",(char*)&response[i]);
            //msg[j] = response[i];

        if (r==1) 
        {
            //dump contents to pipe
            //return (char*) msg; 
        }
    }

    return r;
}

int
SC_dumpBytes(unsigned char *bytes, size_t size) 
{
    // Output outgoing transmission
    fprintf(stdout, "\r\nBytes: ");
    int j=0;
    for (j=0; j<size; j++) 
    {
        printf("%02X ",bytes[j]);
    }    
    fprintf(stdout, "\r\nTotal bytes: %d",(unsigned int)size);
    return 1;
}

/** Provide signal handling for SC_read **/
volatile sig_atomic_t stop;
void inthand (int signum) { stop = 1; }

/**
 * Function to read bytes in single-file from the serial device and 
 * append them to and return a response array
 *
 * !! Serial port is not sending any CR, LF, or other E-O-L or E-O-F bytes
 * !! Must check for Sync bytes and verify they are not in payload, e.g. 'He'llo
 * 
 * 1 - findstart building message frame, checking if each byte makes sense, else discard and look for 48 65
 *
 * @param fdin - the file descriptor representing the serial device
 */
void
SC_read (int fdin) 
{
    //FILE *fdout; 
    //fdout = fopen("/var/log/space/he100.log","a");

    // Read response
    unsigned char buffer[1];
    unsigned char response[255];
    int chars_read;
    int i=0;
    int action=0;
    int breakcond=255;
    
    // Read continuously from serial device
    signal(SIGINT, inthand);
  
    while (!stop)
    {
        if ( chars_read = read(fdin, &buffer, 1) > 0 )// if a byte is read
        { 
            printf("\r\n SC_read: i:%d chars_read:%d buffer:0x%02X",i,chars_read,buffer[0]);

            // set break condition based on incoming byte pattern
            if ( i==4 && (buffer[0] == 0x0A || buffer[0] == 0xFF ) ) // getting an ack
            {
                breakcond=8; // ack is 8 bytes
            }
            else if ( i==5 && breakcond==255 ) // this is the length byte, set break point accordingly
            {

                fprintf(stdout,"\r\n SC_read: Got length byte");
                breakcond = buffer[0] + 10;
            }

            if ( SC_referenceByteSequence(&buffer,i) > 0 ) 
            {
                    fprintf(stdout," >> returned 1");
                    response[i]=buffer[0];
                    buffer[0] = '\0';
                    //fprintf(fdout, "0x%02X ", response[i]);
                    fprintf(
                        stdout,
                        "\n  i:%d breakcondition:%d",
                        i,breakcond
                    );
                    i++;
            }
            else {
                fprintf(stdout," >> returned -1");
                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
            }

            if (i==breakcond) 
            {
                fprintf(stdout,"\n SC_read: hit break condition!");
                if (i>0) // we have a message to validate 
                {
                    if ( SC_validateResponse(response, breakcond) > 0 ) {
                        fprintf(stdout, "\r\n VALID MESSAGE!");
                        // SC_storeData(response, i+1);
                    }
                    else {
                        fprintf(stderr, "\r\n Invalid data!\r\n");
                        SC_dumpBytes(response, i+1);
                    }
                }

                i=0; // restart message index
                response[0] = '\0';
                breakcond=255;
                //fprintf(fdout,"\n");
            }

            buffer[0] = '\0'; // wipe buffer each time
        }
    }
}

/**
 * Function to prepare data for transmission
 * @param char payload - data to be transmitted
 * @param size_t length - length of data stream
 */
unsigned char* 
SC_prepareTransmission(
    unsigned char *payload, 
    size_t length, 
    unsigned char *command
)
{
    unsigned char *transmission = (char *) malloc(length+10);
    unsigned char *payloadbytes = (char *) malloc(length+8);
    
    // attach sync bytes to final transmission byte array
    transmission[0] = SYNC1; //0x48;
    transmission[1] = SYNC2; //0x65;

    // attach command bytes to intermediary payload byte array
    payloadbytes[0] = (unsigned char) command[0];
    payloadbytes[1] = (unsigned char) command[1];

    // attach length bytes
    payloadbytes[2] = 0x00;
    payloadbytes[3] = (unsigned char) length & 0xff; 

    // generate and attach header checksum
    SC_checksum header_checksum = SC_fletcher16(payloadbytes,4); 
    payloadbytes[4] = (unsigned char) header_checksum.sum1 & 0xff;
    payloadbytes[5] = (unsigned char) header_checksum.sum2 & 0xff;
    printf ("\r\nheader_checksum: [%d,%d], [%d,%d]",
        header_checksum.sum1, header_checksum.sum2,
        payloadbytes[4], payloadbytes[5]);
    
    // generate and attach payload checksum
    //SC_checksum payload_checksum = SC_fletcher16(payload,length); // chksum only payload
    SC_checksum payload_checksum = SC_fletcher16(payloadbytes,length+6); // chksum everything except 'He'
    payloadbytes[6+length] = payload_checksum.sum1;
    payloadbytes[6+length+1] = payload_checksum.sum2;
    printf ("\r\npayload_checksum: [%d,%d], [%d,%d]",
        payload_checksum.sum1, payload_checksum.sum2,
        payloadbytes[6+length], payloadbytes[6+length+1]);

    // attach data to payload 
    int i;
    for (i=0;i<length;i++)
        payloadbytes[6+i] = payload[i];
    int j=0;

    // attach payload and return final transmission
    for (i=2;i<length+10;i++) {
        transmission[i] = payloadbytes[j];
        j++;
    }

    return (char*) transmission;
}

int
SC_referenceByteSequence(unsigned char *response, int position)
{
    printf("\r\n  SC_referenceByteSequence(0x%02X,%d)",*response,(int)position);
    int r = -1;
    switch ((int)position)
    {
        case 0   : // first position should be 0x48 : 72
                if ((int)*response==72) r=1;
                //else if ((*response)==NULL) return -1;
                break;
        case 1   : // second position should be 0x65 : 101 
                if ((int)*response==101) r=1;
                break;
        case 2   : // response tx/rx command should be 0x20
                if ((int)*response==32) r=1; // CMD_RECEIVE  0x20
                break;
        case 3   : // response command could be between 1-20
                if (*response > 0x00 && *response < 0x20) r=(int)*response; 
                break;
        case 4   : // first length byte
                if ( 
                       *response == 0x00 // start of length byte
                    || *response == 0x0A // start of ack, should match 5
                    || *response == 0xFF // start of noack, should match 5
                   ) r=1;
                break;
        //case 5   : // real length byte
                //if ((int)*response<MAX_FRAME_LENGTH) r=(int)*response; 
                //break;
        default  : r=1; break;
    }
    return r; 
}

/** 
 * Function to decode validated and extracted data from response
 * @param response - the response data to interpret
 * @param length - the length of the data in bytes
 *
 * Most responses are 8 bytes ack or no ack 
 * noop_ack         486520010a0a35a1
 * noop_noack       48652001ffff1f80
 * setconfig_ack    486520060a0a3ab0
 * tx_ac            486520030a0a37a7
 */
int
SC_interpretResponse (char *response, size_t length) 
{
    printf("Response length: %d\n", (int)length);

    // if response == transmission, He100 device is off!   
    if ((int)response[2]==16) {
        fprintf(stderr,"SC_read: He100 is off!");
        return 0;
    }

    char* value; 
    int i=0;
    for (i=0; i<length; i++) // only runs once! 
    {
        printf("0x%02X : %d ",response[i], response[i]);
        // instead of this, build array from struct and lookup based on position and value
        // log failed transmissions with shakespeare
        switch ((int)response[i])
        {
            case 72  : 
                value = (i==0) ? "Sync H" : "data"; break;
            case 101 : 
                value = (i==1) ? "Sync e" : "data"; break;
            case 32  : 
                value = (i==2) ? "Response" : "data"; break;
            case 3   : 
                value = (i==3) ? "Transmission" : "data"; break;
            case 10  : 
                value = (i==4||i==5) ? "Acknowledge" : "data"; break;
            default  : 
                value = (i==length||i==length-1) ?"chksum" :"data"; break;
        }
        printf(": %s,\n",value);
    }
    return 1;
}

/**
 * Function to return NOOP byte sequence 
 * no arguments 
 */
unsigned char *
SC_NOOP ()
{
   //noop[10] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43,0x00,0x00};
   unsigned char noop_payload[1] = {0};
   unsigned char noop_command[2] = {CMD_TRANSMIT, CMD_NOOP};
   return SC_prepareTransmission(noop_payload, 0, noop_command);
}

/**
 * Function returning byte sequence to enable beacon on given interval 
 * int beacon_interval interval in seconds 
 */
unsigned char *
SC_setBeaconInterval (int beacon_interval)
{
   unsigned char beacon_interval_payload[1] = {beacon_interval};
   unsigned char beacon_interval_command[2] = {CMD_TRANSMIT, CMD_BEACON_CONFIG};
   return SC_prepareTransmission(beacon_interval_payload, 1, beacon_interval_command);
}

/**
 * Function returning byte sequence to set the beacon message 
 * unsigned char *beacon_message_payload message to transmit 
 */
unsigned char *
SC_setBeaconMessage (unsigned char *beacon_message_payload, size_t beacon_message_len)
{
   unsigned char beacon_message_command[2] = {CMD_TRANSMIT, CMD_BEACON_DATA};
   return SC_prepareTransmission(beacon_message_payload, beacon_message_len, beacon_message_command);
}

/**
 * Function returning byte sequence to amplify power based
 * on input int power_level
 * int power_level decimal value from 0-255 (0%-100%)
 */
unsigned char *
SC_fastSetPA (int power_level)
{
   unsigned char PA_payload[1] = {power_level};
   unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
   return SC_prepareTransmission(PA_payload, 1, fast_set_pa_command);
}

/**
 * Function returning byte sequence to soft reset HE100 board and restore flash settings 
 * no arguments
 */
unsigned char *
SC_softReset()
{
   unsigned char soft_reset_payload[1] = {0}; // careful NPE!
   unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
   return SC_prepareTransmission(soft_reset_payload, 0, soft_reset_command);
}
