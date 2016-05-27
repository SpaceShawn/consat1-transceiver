#! /usr/bin/env python

from array import *
import time, sys, serial
from transclib import *
import ConfigParser
import signal # for interrupt handler

from threading import Thread
import time

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
  port='/dev/ttyUSB0',
  baudrate=9600,
  parity=serial.PARITY_NONE,
  bytesize=serial.EIGHTBITS,
  stopbits=serial.STOPBITS_ONE,
  timeout=1,
  writeTimeout=3
  )

def signal_handler(signal, frame):
  print '\r\nYou pressed Cntl+C! Exiting Cleanly...'
  ser.close()
  sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

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
    if ((response == '486520060a0a3ab0') | (response == '486520010a0a35a1') | (response == '486520030a0a37a7') | (response == '486520200a0a54fe')):
      print 'Acknowledge'
    elif (response == '48652001ffff1f80'):
      print 'Not-Acknowledge'
    elif (bytearray.fromhex(response) == input):
      print 'Serial device is off'
  else :
    print 'You suck'
  print '\r'

def SC_printMenu():
  print 'conv - k - enter conversation mode \r'
  print 'checksum - cs - return a checksum of the entered text \r'
  print 'noop - np - send no-op sequence\r'
  print 'digipeat - dp - digipeater mode\r'
  print 'listen - l - listen for incoming communication\r'
  print 'getconfig- gc - send getConfig\r'
  print 'getfirmware- gf - get firmware revision\r'
  print 'looptransmit - lt\r'
  print 'transmit - t - transmit given data\r'
  print 'testtransmit - tt - transmit hard-coded data\r'
  print 'setbaud - sbaud - set the BAUD from 9600 to 38400\r'
  print 'setbeacon - sbeacon - set the Beacon rate from input\r'
  print 'setledpulse - slp - set the LED to pulse every 2.5 seconds\r'
  print 'setconfig - sc - set the radio config\r'
  print 'setpoweramp - spa - set the power amplification based on input'
  print 'writeflash - wf - stores the current configuration in radio flash'
  print 'exit - q - and close the serial port\r\n'

#  print "error opening serial port: " + str(e)
#  exit()

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
    if ( (response[4] == '0a') and (response[5] == '0a') ):
      print 'Acknowledge'
    elif ((response[4] == 'ff') and (response[5] == 'ff') ):
      print 'Not-Acknowledge'
    elif (bytearray.fromhex(response) == input):
      print 'Serial device is off'
  else :
    print 'Unknown problem, maybe radio is off'
  print '\r'

def SC_transmitPrompt():
  while True:
    print 'Enter a message to transmit'
    input=raw_input()
    if len(input) == 0:
        input = " "
    SC_writeCallback(SC_transmit(input))

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

    elif ((input == "conv") | (input == "k")):
      ta = Thread( target=SC_transmitPrompt() )
      tb = Thread( target=SC_listen(ser) )

      ta.start()
      tb.start()
    
      ta.join()
      tb.join()

    elif ((input == "digipeat") | (input == "dp")):
      SC_digipeat(ser)
       
    elif ((input == "listen") | (input == "l")):
      SC_listen(ser)

    elif ((input == "getconfig") | (input == "gc")):
      input = SC_getConfig()
      SC_writeCallback(input)

    elif ((input == "noop") | (input == "np")) :
      input = SC_noop()
      SC_writeCallback(input)

    elif ((input == "transmit") | (input == "t")):
      SC_transmitPrompt()

    elif ((input == "testtransmit") | (input == "tt")):
      input = SC_testTransmit()
      SC_writeCallback(input)

    elif ((input == "setbaud") | (input == "sbaud")):
      input=SC_setLED()
      SC_writeCallback(input)

    elif ((input == "getfirmware") | (input == "gf")):
      input=SC_getFirmware()
      #SC_writeCallback(SC_prepare("", "10 12"))
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

    elif ((input == "setpoweramp") | (input == "spa")):
      print 'Enter a power amplification level 0-100%'
      input=raw_input()

      try:
        pa_level = int(input)
      except ValueError:
        print "incorrect input"

      if pa_level <= 100 | pa_level >= 0:
        print 'Setting power amplification level: ' + str(pa_level) + "%"
        print 'PA%: ' + str(pa_level)
        pa_level = int(float(pa_level)/100 * 255)
        print 'Dec: ' + str(pa_level)
        print 'Hex: ' + str(hex(pa_level))
        byte = struct.pack('B',pa_level)
        SC_writeCallback(SC_prepare(byte, "10 20"))
      else:
        print "incorrect input"

    elif ((input == "setconfig") | (input == "sc")):
      input=SC_setConfig()
      SC_writeCallback(input)

    elif ((input == "looptransmit") | (input == "lt")):
      payload="A123456789B123456789C123456789D123456789E123456789F123456789G123456789H123456789I123456789J123456789K123456789L123456789M123456789N123456789O123456789P123456789Q123456789R123456789S"
      input=SC_prepare(payload.encode('utf-8'), '10 03')
      while True:
        #action = raw_input(": ")
        SC_writeCallback(input)
        #if action == 'q':
         # print "Stopping looping transmit"
          #break
        out = ''
        while ser.inWaiting() > 0:
          out += ser.read(1)
        if out != '':
          print "Got a response"

    elif ((input == "dectohex") | (input == "d2h")):
      print 'Enter a message to convert'
      input=raw_input()
      print hex(int(input))

    elif ((input == "setledpulse") | (input == "slp")):
      input=SC_setLEDPulse()
      SC_writeCallback(input)

    elif ((input == "checksum") | (input == "cs")):
      print 'Enter a message to checksum'
      input=raw_input()
      print '8-bit', SC_fletcher8(input)
#      print SC_fletcher16(input)
      print '32-bit', SC_fletcher32(input)
    elif ((input == "writeflash") | (input == "wf")):
      input=SC_writeFlash()
      SC_writeCallback(input)
    else:
      SC_printMenu()

#	except Exception, e1:
#		print "error communicating...:" + str(e1)
else :
  ser.open()
	#except Exception, e:
	#print "Cannot open serial port"
