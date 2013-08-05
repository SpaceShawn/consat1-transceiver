import serial

ser = serial.Serial(
#    3,
  port='/dev/ttyUSB0',
  baudrate=9600,
  parity=serial.PARITY_NONE,
  bytesize=serial.EIGHTBITS,
  stopbits=serial.STOPBITS_ONE,
  timeout=1,
  writeTimeout = 3
  )

#  print "error opening serial port: " + str(e)
#  exit()

