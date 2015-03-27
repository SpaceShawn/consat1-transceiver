#ifndef SC_SERIAL_H_
#define SC_SERIAL_H_

/**
 * Function to configure serial interface
 * @param fdin - the file descriptor representing the serial device
 * @return int - exit status 
 * REF: http://man7.org/linux/man-pages/man3/termios.3.html
 * REF: http://www.unixguide.net/unix/programming/3.6.2.shtml
 */
int SC_configureInterface (int);

/* Function to open HE100 device on configured seial port address */
int SC_openPort (void);

/* Function to close serial device connection at given file descriptor */
int SC_closePort (int);

#endif
