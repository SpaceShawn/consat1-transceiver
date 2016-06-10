#! /usr/bin/env python

from array import *
import time, sys, serial
from transclib_aleksandr import *
import signal # for interrupt handler

from threading import Thread
import time

def SC_writeCallback(inp, ser):
  try:
    ser.write(inp)
  except(Exception, e):
    print("Could not write for reasons")

  out = ''
  time.sleep(1);

  while ser.inWaiting() > 0:
    out += ser.read(1)

  if out!= '':
    print("Response:")
    response = toHex(out)
    print(response)
    if ((response == '486520060a0a3ab0') | (response == '486520010a0a35a1') | (response == '486520030a0a37a7') | (response == '486520200a0a54fe')):
      print("Acknowledge")
    elif (response == '48652001ffff1f80'):
      print("Not-Acknowledge")
    elif (bytearray.fromhex(response) == inp):
      print("Serial device is off")
  else :
    print("You suck")
  print("\r")

def init_transceiver():
  ser = serial.Serial(
    port='/dev/ttyS1',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    bytesize=serial.EIGHTBITS,
    stopbits=serial.STOPBITS_ONE,
    timeout=1,
    writeTimeout=3
    )
  return ser

def signal_handler(signal, frame):
  print("\r\nYou pressed Cntl+C! Exiting Cleanly...")
  ser.close()
  sys.exit(0)
#signal.signal(signal.SIGINT, signal_handler)

def SC_printMenu():
  print("conv - k - enter conversation mode \r")
  print("checksum - cs - return a checksum of the entered text \r")
  print("noop - np - send no-op sequence\r")
  print("digipeat - dp - digipeater mode\r")
  print("listen - l - listen for incoming communication\r")
  print("getconfig- gc - send getConfig\r")
  print("getfirmware- gf - get firmware revision\r")
  print("looptransmit - lt\r")
  print("transmit - t - transmit given data\r")
  print("testtransmit - tt - transmit hard-coded data\r")
  print("setbaud - sbaud - set the BAUD from 9600 to 38400\r")
  print("setbeacon - sbeacon - set the Beacon rate from input\r")
  print("setledpulse - slp - set the LED to pulse every 2.5 seconds\r")
  #print("setconfig - sc - set the radio config\r")
  print("setpoweramp - spa - set the power amplification based on input")
  #print("writeflash - wf - stores the current configuration in radio flash")
  print("exit - q - and close the serial port\r\n")

#  print "error opening serial port: " + str(e)
#  exit()

def SC_writeCallback(inp):
  ser.write(inp)
  out = ''	
  time.sleep(1);

  while ser.inWaiting() > 0:
    out += ser.read(1)
    
  if out!= '':
    response = toHex(out)
    print("Response: ", response)
    if ( (response[4] == '0a') and (response[5] == '0a') ):
      print("Acknowledge")
    elif ((response[4] == 'ff') and (response[5] == 'ff') ):
      print("Not-Acknowledge")
    elif (bytearray.fromhex(response) == inp):
      print("Serial device is off")
  else :
    print("Unknown problem, maybe radio is off")
  print("\r")

def SC_transmitPrompt():
  while True:
    print("Enter a message to transmit")
    inp=input()
    if len(inp) == 0:
        inp = " "
    SC_writeCallback(SC_transmit(inp))
