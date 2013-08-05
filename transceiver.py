from array import *
import time, sys, serial
from transclib import *

ser = serial.Serial(
    3,
  baudrate=9600,
  parity=serial.PARITY_NONE,
  bytesize=serial.EIGHTBITS,
  stopbits=serial.STOPBITS_ONE,
  timeout=1,
  writeTimeout = 3
  )

#  print "error opening serial port: " + str(e)
#  exit()

if ser.isOpen():
  print 'Enter commands for the transceiver below.\r\nType "menu" for predefined commands, and "exit" to quit'
  input=1
  while 1:
    input=raw_input(">> ")
    if input == "exit":
      ser.close()
      exit()
    elif input == "menu":
      print 'noop - send no-op sequence\r'
      print 'getconfig - send getConfig\r'
      print 'setconfig - send setconfig\r'
      print 'transmit - transmit hard-coded data\r\n'

    elif input == "noop":
      input = SC_noop()
      #ser.write(input + '\r\n')
      ser.write(input)
      out = ''	
      time.sleep(1);

      while ser.inWaiting() > 0:
	out += ser.read(1)

      if out!= '':
	print toHex(out)
	print ('>>')
    elif input == "transmit":
      print 'Enter a message to transmit'
      input=raw_input()

      ser.write(SC_transmit(input))
      print 'Response'

      out = ''	
      time.sleep(1);

      while ser.inWaiting() > 0:
		out += ser.read(1)

      if out!= '':
		print toHex(out)
		print ('>>')
    elif input == "checksum":
      print 'Enter a message to checksum'
      input=raw_input()
      print SC_fletcher8(input)
#      print SC_fletcher16(input)
      print SC_fletcher32(input)
    else:
      ser.write(input + '\r\n')
      out = ''	
      time.sleep(1);

      while ser.inWaiting() > 0:
	out += ser.read(1)

	if out!= '':
	  print ">>" + out
#	except Exception, e1:
#		print "error communicating...:" + str(e1)
else :
  ser.open()
	#except Exception, e:
	#print "Cannot open serial port"
