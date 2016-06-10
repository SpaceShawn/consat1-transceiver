#! /usr/bin/env python
import sys
sys.path.append('/root/csdc3/src/logs')
sys.path.append('/root/csdc3/src/sensors')
sys.path.append('/root/csdc3/src/utility')

from array import *
import time, sys, serial
from transclib_aleksandr import *
import signal # for interrupt handler

from threading import Thread
import time
from chomsky import selectTelemetryLog
from sensor_constants import *
from ast import literal_eval
import datetime
from utility import get_disk_usage
from utility import get_uptime

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

ser = serial.Serial(
  port='/dev/ttyS1',
  baudrate=9600,
  parity=serial.PARITY_NONE,
  bytesize=serial.EIGHTBITS,
  stopbits=serial.STOPBITS_ONE,
  timeout=1,
  writeTimeout=3
)

def signal_handler(signal, frame):
  print("\r\nYou pressed Cntl+C! Exiting Cleanly...")
  ser.close()
  sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

def SC_printMenu():
  print("conv - k - enter conversation mode \r")
  print("checksum - cs - return a checksum of the entered text \r")
  print("noop - np - send no-op sequence\r")
  print("digipeat - dp - digipeater mode\r")
  print("beacon - b - start beacon mode\r")
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

#  print("error opening serial port: " + str(e)
#  exit()

def SC_writeCallback(inp):
  ser.write(inp)
  out = ''
  time.sleep(1);
  out = ser.read(1)

  while ser.inWaiting() > 0:
    out += ser.read(1)

  if out!= '':
    out = str(out)
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

if ser.isOpen():
  print("Enter commands for the transceiver below.\r\nType menu for predefined commands, and exit to quit")
  inp=1
  while 1:
    inp=input(">> ")

    if ((inp == "exit") | (inp == "q")):
      ser.close()
      exit()

    elif inp == "menu":
      SC_printMenu()

    elif ((inp == "conv") | (inp == "k")):
      ta = Thread( target=SC_transmitPrompt() )
      tb = Thread( target=SC_listenLoop(ser) )
      ta.start()
      tb.start()
      while True:
          ta.join()
          tb.join()

    elif ((inp == "digipeat") | (inp == "dp")):
      while True:
        message = SC_listen(ser)
        if message != None:
            print("digipeat: ", message)
            SC_writeCallback(SC_transmit(message))

    elif ((inp == "beacon") | (inp == "b")):
        for i in range(5):
            curr_time = int(time.time())
            power_tuple = selectTelemetryLog(POWER)
            cdh_brd_temp = selectTelemetryLog(TEMP_CDH_BRD)
            temp = cdh_brd_temp[0][1]

            power = literal_eval(power_tuple[0][1])
            timestamp = power_tuple[0][3]
            print(timestamp)
            voltage = int(power[0]) / 1000.

            uptime = get_uptime
            freespace = get_disk_usage('/') / 1000000.

            date = datetime.datetime.fromtimestamp( curr_time ).strftime('%H:%M:%S %Y-%m-%d')
            SC_writeCallback(SC_transmit("VBAT:%.2f | CDH_TEMP:%s | UPTIME: %s | FREESPACE | %.2f | %s" \
                                      % (voltage, temp, uptime, freespace, date)))
            time.sleep(5)

    elif ((inp == "listen") | (inp == "l")):
      while True:
        message = SC_listen(ser)
        if message != None:
            print("received: ", message)

    elif ((inp == "getconfig") | (inp == "gc")):
      inp = SC_getConfig()
      SC_writeCallback(inp)

    elif ((inp == "noop") | (inp == "np")) :
      inp = SC_noop()
      SC_writeCallback(inp)

    elif ((inp == "transmit") | (inp == "t")):
      SC_transmitPrompt()

    elif ((inp == "testtransmit") | (inp == "tt")):
      inp = SC_testTransmit()
      SC_writeCallback(inp)

    elif ((inp == "setbaud") | (inp == "sbaud")):
      inp=SC_setLED()
      SC_writeCallback(inp)

    elif ((inp == "getfirmware") | (inp == "gf")):
      inp=SC_getFirmware()
      #SC_writeCallback(SC_prepare("", "10 12"))
      SC_writeCallback(inp)

    elif ((inp == "setbeacon") | (inp == "sbeacon")):
      print("Enter a beacon level from (0-3)")
      inp=input()

      try:
        beacon_level = int(inp)
      except(ValueError):
        print("incorrect input")

      if beacon_level < 4 | beacon_level >= 0:
        SC_writeCallback(SC_beacon(inp))
      else:
        print("incorrect inp")

    elif ((inp == "setpoweramp") | (inp == "spa")):
      print("Enter a power amplification level 0-100%")
      inp=input()

      try:
        pa_level = int(inp)
      except(ValueError):
        print("incorrect input")

      if pa_level <= 100 | pa_level >= 0:
        print("Setting power amplification level: ", str(pa_level), "%")
        print("PA%:", str(pa_level))
        pa_level = int(float(pa_level)/100 * 255)
        print("Dec: ", str(pa_level))
        print("Hex: ", str(hex(pa_level)))
        byte = struct.pack('B',pa_level)
        SC_writeCallback(SC_prepare(byte, "10 20"))
      else:
        print("incorrect input")

    elif ((inp == "setconfig") | (inp == "sc")):
      inp=SC_setConfig()
      SC_writeCallback(inp)

    elif ((inp == "looptransmit") | (inp == "lt")):
      payload="A123456789B123456789C123456789D123456789E123456789F123456789G123456789H123456789I123456789J123456789K123456789L123456789M123456789N123456789O123456789P123456789Q123456789R123456789S"
      inp=SC_prepare(payload.encode('utf-8'), '10 03')
      while True:
        #action = input(": ")
        SC_writeCallback(inp)
        #if action == 'q':
         # print("Stopping looping transmit"
          #break
        out = ''
        while ser.inWaiting() > 0:
          out += ser.read(1)
        if out != '':
          print("Got a response")

    elif ((inp == "dectohex") | (inp == "d2h")):
      print("Enter a message to convert")
      inp=input()
      print(hex(int(inp)))

    elif ((inp == "setledpulse") | (inp == "slp")):
      inp=SC_setLEDPulse()
      SC_writeCallback(inp)

    elif ((inp == "checksum") | (inp == "cs")):
      print("Enter a message to checksum")
      inp=input()
      print("8-bit", SC_fletcher8(inp))
#      print SC_fletcher16(input)
      print("32-bit", SC_fletcher32(inp))
    elif ((inp == "writeflash") | (inp == "wf")):
      inp=SC_writeFlash()
      SC_writeCallback(inp)
    else:
      SC_printMenu()

else :
  try:
    ser.open()
  except(Exception, e):
    print("Cannot open serial port")
