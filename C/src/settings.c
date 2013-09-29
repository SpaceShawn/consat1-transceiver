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
