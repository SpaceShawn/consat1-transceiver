from math import *
import struct
import sys
import signal # for interrupt handler

from itertools import islice, izip

def signal_handler(signal, frame):
  print 'You pressed Cntl+C! Exiting Cleanly...'
  ser.close()
  sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

def SC_computeFletcher(data, size, modulo, limit=None):
	#valA, valB = 0xf, 0xf
	#valA, valB = 0xf, 0xf
	valA, valB = 0, 0

	length = len(data)	
	if isinstance(data, str):
		if limit is not None and length > limit:
			data = data[:limit]
		for char in data:
			valA += (ord(char) << 8) # & 0xf
			valB += valA
		valA %= modulo 
		valB %= modulo 
	else:
		if limit is not None and length > limit:
			data = data[:limit]
		for c in data:
			valA += (c << 8) #& 0xf
			valB += valA
		valA %= modulo 
		valB %= modulo 
	
	return (valA, valB)
#	return (valB << (size/2)) + valA

def fletcher_checksum(data): 
    sum1 = 0
    sum2 = 0
    
    length = len(data)
    for byte1, byte2 in izip(islice(data, 0, length, 2),
                            islice(data, 1, length, 2)):
        sum1 = sum1 + ((ord(byte1) << 8) | ord(byte2))
        sum2 = (sum2 + sum1)
    
    if length % 2 == 1:
        sum1 = sum1 + (ord(data[-1]) << 8)
        sum2 = sum1 + sum2
    
    while sum1 > 0xFFFF:
        sum1 = (sum1 & 0xFFFF) + (sum1 >> 16)
    
    while sum2 > 0xFFFF:
        sum2 = (sum2 & 0xFFFF) + (sum2 >> 16)
    
    #return (sum2 << 16) | sum1
    return (sum2, sum1)

def SC_fletcher8(data):
  return SC_computeFletcher(data, 8, 255, limit=21)

def SC_fletcher32(data):
  return SC_computeFletcher(data, 32, 65535, limit=359)

def SC_fletcher(data):
  return SC_fletcher8(data)
 # return fletcher_checksum(data)

def SC_noop(): 
  return bytearray.fromhex('48 65 10 01 00 00 11 43 00 00')

def SC_getConfig(): 
##  return bytearray.fromhex('48 65 10 04 00 00 14 4c 00 00')
  return bytearray.fromhex('48 65 10 05 00 00 15 4f') #00 00

def SC_setLED():
  return bytearray.fromhex('48 65 10 06 00 22 38 74 00 00 01 01 00 00 48 33 02 00 98 93 06 00 56 41 33 4f 52 42 56 45 32 43 55 41 09 00 00 00 41 00 06 00 37 7c')

# function to allow enable simultaneous transmission and receiving
def SC_converse():
  return false;

def SC_listen():
  while True:
    action = raw_input("=> ")
    if action == 'q':
      print "Stopping listener"
      break

    while ser.inWaiting() > 0:
      out += ser.read(1)
#      out = ser.readline()    
    if out!= '':
      print 'Incoming:'    
      response = toHex(out)

def SC_setBAUD():
  return bytearray.fromhex('48 65 10 05 00 2e 43 7d 01 00 00 00 00 00 00 00 02 04 02 06 04 a 00 00 20 a7 06 00 80 32 02 00 4e 4f 43 41 4c 4c 4e 4f 43 41 4c 4c a 64 60 00 00 00 00 00 00 00 de 35')

def SC_testTransmit():
## weirdness: "A payload checksum is then used to verify the accuracy of the payload. The checksum is calculated across all pertinent bytes of the message excluding the two sync characters of each message 'He'
## possible interpretation:
### header checksum is of header minus sync bytes, payload checksum is of total minus sync bytes
### header checksum is of header minus sync bytes, payload checksum is of payload minus sync bytes
### header checksum is of header in full, payload checksum is of total minus sync bytes
###  payload = '12345678'
##  print '\r\n>> Hardcoded payload: "12345678"'  return bytearray.fromhex('48 65 10 03 0a 1d 53 64 65 66 67 68 69 6a 6b 6c 6d 15 21')
  return bytearray.fromhex('48 65 10 03 00 FF 12 48 41 31 32 33 34 35 36 37 38 39 42 31 32 33 34 35 36 37 38 39 43 31 32 33 34 35 36 37 38 39 44 31 32 33 34 35 36 37 38 39 45 31 32 33 34 35 36 37 38 39 46 31 32 33 34 35 36 37 38 39 47 31 32 33 34 35 36 37 38 39 48 31 32 33 34 35 36 37 38 39 49 31 32 33 34 35 36 37 38 39 4a 31 32 33 34 35 36 37 38 39 4b 31 32 33 34 35 36 37 38 39 4c 31 32 33 34 35 36 37 38 39 4d 31 32 33 34 35 36 37 38 39 4e 31 32 33 34 35 36 37 38 39 4f 31 32 33 34 35 36 37 38 39 50 31 32 33 34 35 36 37 38 39 51 31 32 33 34 35 36 37 38 39 52 31 32 33 34 35 36 37 38 39 53 31 32 33 34 35 36 37 38 39 54 31 32 33 34 35 36 37 38 39 55 31 32 33 34 35 36 37 38 39 56 31 32 33 34 35 36 37 38 39 57 31 32 33 34 35 36 37 38 39 58 31 32 33 34 35 36 37 38 39 59 5a 31 32 33 34 aa 4b')

def SC_transmit(payload): 
  #payload="A123456789B123456789C123456789D123456789E123456789F123456789G123456789H123456789I123456789J123456789K123456789L123456789M123456789N123456789O123456789P123456789Q123456789R123456789S123456789T123456789U123456789V123456789W123456789X123456789Y123456789Z1234"
  payload_byte_array = payload.encode('utf-8')
  #payload_byte_array = bytearray.fromhex('41 31 32 33 34 35 36 37 38 39 42 31 32 33 34 35 36 37 38 39 43 31 32 33 34 35 36 37 38 39 44 31 32 33 34 35 36 37 38 39 45 31 32 33 34 35 36 37 38 39 46 31 32 33 34 35 36 37 38 39 47 31 32 33 34 35 36 37 38 39 48 31 32 33 34 35 36 37 38 39 49 31 32 33 34 35 36 37 38 39 4a 31 32 33 34 35 36 37 38 39 4b 31 32 33 34 35 36 37 38 39 4c 31 32 33 34 35 36 37 38 39 4d 31 32 33 34 35 36 37 38 39 4e 31 32 33 34 35 36 37 38 39 4f 31 32 33 34 35 36 37 38 39 50 31 32 33 34 35 36 37 38 39 51 31 32 33 34 35 36 37 38 39 52 31 32 33 34 35 36 37 38 39 53 31 32 33 34 35 36 37 38 39 54 31 32 33 34 35 36 37 38 39 55 31 32 33 34 35 36 37 38 39 56 31 32 33 34 35 36 37 38 39 57 31 32 33 34 35 36 37 38 39 58 31 32 33 34 35 36 37 38 39 59 5a 31 32 33 34')
  length = len(payload_byte_array)
  length_bytes = struct.pack('B',length)  
  print "Payload:  ", length, " bytes out of a maximum 255\r\n"

  print 'Keeping 2 bytes separate for sync characters'
  transmission = bytearray.fromhex('48 65')

  print 'Adding 2 bytes for header ...'
  header = '10 03' 
  packet=bytearray.fromhex(header) 
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  print "Adding 2 bytes for payload size ..."
  packet.extend(bytearray.fromhex('00'))
  packet.extend(length_bytes)
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  print "Adding 2 bytes for header checksum ..."  
  header_checksum = SC_fletcher(packet)
  packet.extend(struct.pack('B', header_checksum[0]))
  packet.extend(struct.pack('B', header_checksum[1]))
  print "checksum: ", header_checksum#, " should be: (12,48)" 
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  print "Adding", len(payload_byte_array), "bytes for payload .."
  packet.extend(payload_byte_array)
  print "Message:  ", toHex(str(payload_byte_array))
  print "bytesize:  ", len(payload_byte_array), " bytes\r\n"
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  print "Adding 2 bytes for payload checksum"
  payload_checksum = SC_fletcher(payload)
  packet.extend(struct.pack('B', payload_checksum[0]))
  packet.extend(struct.pack('B', payload_checksum[1]))
  print "checksum: ", payload_checksum#, " should be: (170,75)\r\n"#aa,4b
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  transmission.extend(packet)
  print "total payload: ", len(transmission), " bytes\r\n"
  print 'SENDING:: ', toHex(str(transmission))  
  return transmission

def SC_setConfig():
  return false;

def SC_size(data):
  return str(sys.getsizeof(data))

def unsigned(n):
  return n & 0xFFFFFFFF

def toHex(s):
  lst = []
  for ch in s:
    hv = hex(ord(ch)).replace('0x', '')
    if len(hv) == 1:
      hv = '0'+hv	   
    lst.append(hv)
  return reduce(lambda x,y:x+y, lst)

def toStr(s):
  return s and chr(atoi(s[:2], base=16)) + toStr(s[2:]) or ''

def intToBytes(val, num_bytes):
  return [(val & (0xff << pos*8)) >> pos*8 for pos in range(num_bytes)]
