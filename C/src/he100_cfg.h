struct he100_settings {
  int 	interface_baud_rate; // Radio Interface Baud Rate (9600=0x00)
  int 	tx_power_amp_level; // Tx Power Amp Level (min=0x00, max=0xFF)
  int 	rx_rf_baud_rate; // Radio RX RF Baud Rate (9600=0x00)
  int 	tx_rf_baud_rate; // Radio TX RF Baud Rate (9600=0x00)
  int 	rx_modulation; // (0x00 = GFSK)
  int 	tx_modulation; // (0x00 = GFSK)
  int 	rx_freq; // Channel Tx Frequency 144200
  int 	tx_freq; // Channel Tx Frequency 431000
  char	source_callsign; // VA3ORB, default NOCALL
  char	destination_callsign; // VE2CUA, default CQ
  int	tx_preamble; // AX25 Mode Tx Preamble byte length (0x00 = 20 flags)
  int	tx_postamble; // AX25 Mode Tx Postamble byte length (0x00 = 20 flags)
  int function_config; // Radio Configuration discrete behaviors
  int function_config2; // Radio Configuration discrete behaviors #2
};

// baudrate settings are defined in <asm/termbits.h> from <termios.h>
#define BAUDRATE B9600
#define TTYDEVICE "/dev/ttyS2"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define PARITYBIT ~PARENB // no parity bit
#define BYTESIZE CS8 // 8 data bits
#define STOPBITS ~CSTOPB // 1 stop bit
#define HWFLWCTL ~CRTSCTS // disable hardware flow control

#define CFG_OFF_LOGIC LOW   0x00

// LED config
#define CFG_LED_BYTE 38  // 38th byte in byte array
#define CFG_LED_PS  0x41 // 2.5 second pulse
#define CFG_LED_TX  0x42 // flash on transmit
#define CFG_LED_RX  0x43 // flash on receive

#define SYNC1       0x48
#define SYNC2           0x65
#define CMD_TRANSMIT        0x10
#define CMD_RECEIVE         0x20
#define CMD_TELEMETRY_DUMP  0x30 // pending??
#define CMD_PING_RETURN     0x31
#define CMD_CODE_UPLOAD     0x33 // pending??
#define CMD_TOGGLE_PIN      0x34

#define CMD_NOOP                0x01 // noop command, increments command processing counter
#define CMD_RESET               0x02 // reset radio processors and systems

#define CMD_TRANSMIT_DATA       0x03 // send N number of bytes 
#define CMD_RECEIVE_DATA        0x04 // receive N number of bytes

#define CMD_GET_CONFIG          0x05 // 20 05 prepends actual config data
// EX {48 65 10 05 ... } request config
// EX {48 65 20 05 ... } actual config
#define CMD_SET_CONFIG          0x06 // followed by config bytes
// EX {48 65 10 06 ... checksum }
// EX {48 65 20 06 ... } -> ACK

#define CMD_TELEMETRY           0x07 // query a telemetry frame
// EX {48 65 10 06 ... } query a telemetry frame
// EX {48 65 20 06 ... } receive a telemetry frame

#define CMD_WRITE_FLASH         0x08 // write flash 16 byte MDF

#define CMD_RF_CONFIGURE        0x09 // Low Level RF Configuration

#define CMD_BEACON_DATA         0x10 // Set Beacon Message
#define CMD_BEACON_CONFIG       0x11 // Set Beacon configuration

#define CMD_READ_FIRMWARE_V     0x12 // read radio firmware revision
// EX {48 65 10 12 ... } request revision number
// EX {48 65 20 12 REV } float 4 byte revision number

#define CMD_DIO_KEY_WRITE       0x13
#define CMD_FIRMWARE_UPDATE     0x14
#define CMD_FIRMWARE_PACKET     0x15
#define CMD_FAST_SET_PA         0x20
