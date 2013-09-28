from array import *
import time, sys, serial
from transclib import *
import ConfigParser

config = ConfigParser.ConfigParser()
config.read('transceiver.ini')
print config.sections()

def ConfigSectionMap(section):
	dict1 = {}
	options = config.options(section)
	for option in options:
		try:
			dict1[option] = Config.get(section, option)
			if dict1[option] == -1:
				DebugPrint("skip: %s" % option)	
		except:
			print("exception on %s!" % option)
			dict1[option] = None
	return dict1

# implement so git can ignore the config file and support multiple machines
#print ConfigSectionMap("SERIAL")['port']
#print ConfigSectionMap("SERIAL")['baudrate']
#print ConfigSectionMap("SERIAL")['parity']
#print ConfigSectionMap("SERIAL")['bytesize']
#print ConfigSectionMap("SERIAL")['stopbits']
#print ConfigSectionMap("SERIAL")['timeout']
#print ConfigSectionMap("SERIAL")['writeTimeout']

ser = serial.Serial(
#    3,# dell laptop
  port='/dev/ttyUSB0',# toshiba laptop
  baudrate=9600,
  parity=serial.PARITY_NONE,
  bytesize=serial.EIGHTBITS,
  stopbits=serial.STOPBITS_ONE,
  timeout=1,
  writeTimeout=3
  )

def SC_writeCallback(input):
  ser.write(input)
  out = ''	
  time.sleep(1);

  while ser.inWaiting() > 0:
    out += ser.read(1)
    
  if out!= '':
    print 'Response:'    
    response = toHex(out)
    print response
    if ((response == '486520060a0a3ab0') | (response == '486520010a0a35a1') | (response == '486520030a0a37a7')):
      print 'Acknowledge'
    elif (response == '48652001ffff1f80'):
      print 'Not-Acknowledge'
    elif (bytearray.fromhex(response) == input):
      print 'Serial device is off'
  else :
    print 'You suck'
  print '\r'

def SC_printMenu():
  print 'checksum - cs - return a checksum of the entered text \r'
  print 'noop - np - send no-op sequence\r'
  print 'listen - l - listen for incoming communication\r'
  print 'getconfig- gc - send getConfig\r'
  print 'setconfig - sc - send setconfig to project defaults\r'
  print 'transmit - t - transmit given data\r'
  print 'testtransmit - tt - transmit hard-coded data\r'
  print 'setbaud - sbaud - set the BAUD from 9600 to 38400\r'
  print 'setbeacon - sbeacon - set the Beacon rate from input\r'
  print 'setledpulse - slp - set the LED to pulse every 2.5 seconds\r'
  print 'setledtx - sltx - set the LED to pulse on transmit\r'
  print 'setledrx - slrx - set the LED to pulse on receive\r'
  print 'exit - q - and close the serial port\r\n'

#  print "error opening serial port: " + str(e)
#  exit()

if ser.isOpen():
  print 'Enter commands for the transceiver below.\r\nType "menu" for predefined commands, and "exit" to quit'
  input=1
  while 1:
    input=raw_input(">> ")

    if ((input == "exit") | (input == "q")):
      ser.close()
      exit()

    elif input == "menu":
      SC_printMenu()

    elif ((input == "listen") | (input == "l")):
      SC_listen(ser) 

    elif ((input == "getconfig") | (input == "gc")):
      input = SC_getConfig()
      SC_writeCallback(input)

    elif ((input == "noop") | (input == "np")) :
      input = SC_noop()
      SC_writeCallback(input)

    elif ((input == "transmit") | (input == "t")):
      print 'Enter a message to transmit'
      input=raw_input()
      SC_writeCallback(SC_transmit(input))

    elif ((input == "testtransmit") | (input == "tt")):
      input = SC_testTransmit()
      SC_writeCallback(input)

    elif ((input == "setbaud") | (input == "sbaud")):
      input=SC_setLED()
      SC_writeCallback(input)

    elif ((input == "setbeacon") | (input == "sbeacon")):
      print 'Enter a beacon level from (0-3)'
      input=raw_input()

      try:
        beacon_level = int(input)
      except ValueError:
        print "incorrect input"

      if beacon_level < 4 | beacon_level >= 0:
        SC_writeCallback(SC_beacon(input))
      else:
        print "incorrect input"

    elif ((input == "setconfig") | (input == "sc")):
      input=SC_setConfig()
      SC_writeCallback(input)

    elif ((input == "setledpulse") | (input == "slp")):
      input=SC_setLEDPulse()
      SC_writeCallback(input)

    elif ((input == "setledtx") | (input == "sltx")):
      input=SC_setLEDTx()
      SC_writeCallback(input)

    elif ((input == "setledrx") | (input == "slrx")):
      input=SC_setLEDRx()
      SC_writeCallback(input)

    elif ((input == "checksum") | (input == "cs")):
      print 'Enter a message to checksum'
      input=raw_input()
      print '8-bit', SC_fletcher8(input)
#      print SC_fletcher16(input)
      print '32-bit', SC_fletcher32(input)
    else:
      SC_printMenu() 

#	except Exception, e1:
#		print "error communicating...:" + str(e1)
else :
  ser.open()
	#except Exception, e:
	#print "Cannot open serial port"
