from math import *
import struct
import sys
import binascii # to convert incoming messages to ascii
from itertools import islice, izip

debug_flag = False

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

def SC_getFirmware(): 
  return bytearray.fromhex('48 65 10 12 00 00 22 76 00 00')

def SC_getConfig(): 
##  return bytearray.fromhex('48 65 10 04 00 00 14 4c 00 00')
  return bytearray.fromhex('48 65 10 05 00 00 15 4f') #00 00

# function to allow enable simultaneous transmission and receiving
def SC_converse():
  return false;

def SC_beacon(instance):
  return {
    '0' : bytearray.fromhex('49 65 10 11 00 01 22 74 00 B8 28'),
    '1' : bytearray.fromhex('48 65 10 11 00 01 22 74 01 B9 29'),
    '2' : bytearray.fromhex('48 65 10 11 00 01 22 74 02 BA 2A'),
    '3' : bytearray.fromhex('48 65 10 11 00 01 22 74 03 BB 2B'),
  }.get(instance, 0)

def SC_listen(ser):
  while True:
    out = ''
    while ser.inWaiting() > 6:
      out += ser.read(6)
    if out != '':
      print "Raw data: ", toHex(out), "\n\r"

      data = bytearray.fromhex(toHex(out))
      payload_length = int(data[5])
      print "Detected payload length" , str(payload_length), "\n\r"
      #payload_length = len(data) - 10

      sleep(1)
      try:
        out += ser.read(payload_length+5)
        data = (bytearray.fromhex(toHex(out)))
      except BufferError:
        print "Incomplete buffer: ", toHex(out), "\n\r"

      print "Raw data: ", toHex(out), "\n\r"
      
      payload = "" 
      header_length = 8
      preamble_length = 19
      for i in xrange(header_length+preamble_length,payload_length-preamble_length) :
        print "0x", str(data[i]), " "
        #payload += chr(data[i])
        payload += chr(out[i])

      expected_frame_length = payload_length + 10
      frame_length = len(data)

      if frame_length != expected_frame_length:
          print "WARNING: mismatched frame_length. Expected:",expected_frame_length, "Actual:",frame_length
      else: 
          print "SUCCESS: frame_length matches expected_frame_length! Expected:",expected_frame_length, "Actual:",frame_length 
      #payload = payload.strip().decode('hex')
       
      print "\n<< Incoming:\n", toHex(out), "\n", toHex(out.strip()).decode('hex'), "\n"
      print "Sync Bytes: ", chr(data[0]), chr(data[1]), "\r"
      print "Command Type: ", str(hex(data[2])), str(hex(data[3])), "\r"
      print "Payload Size: ", data[4], str(int(data[5])), "\r"
      print "Header Check: ", data[6], data[7], "\r"
      print "Payload Check: ", data[len(data)-2], data[len(data)-1], "\r\n\r\n"
      print "Payload: ", str(payload),"\r\n"

def SC_testTransmit():
## weirdness: "A payload checksum is then used to verify the accuracy of the payload. The checksum is calculated across all pertinent bytes of the message excluding the two sync characters of each message 'He'
## possible interpretation:
### header checksum is of header minus sync bytes, payload checksum is of total minus sync bytes
### header checksum is of header minus sync bytes, payload checksum is of payload minus sync bytes
### header checksum is of header in full, payload checksum is of total minus sync bytes
###  payload = '12345678'
##  print '\r\n>> Hardcoded payload: "12345678"'  return bytearray.fromhex('48 65 10 03 0a 1d 53 64 65 66 67 68 69 6a 6b 6c 6d 15 21')
  return bytearray.fromhex('48 65 10 03 00 FF 12 48 41 31 32 33 34 35 36 37 38 39 42 31 32 33 34 35 36 37 38 39 43 31 32 33 34 35 36 37 38 39 44 31 32 33 34 35 36 37 38 39 45 31 32 33 34 35 36 37 38 39 46 31 32 33 34 35 36 37 38 39 47 31 32 33 34 35 36 37 38 39 48 31 32 33 34 35 36 37 38 39 49 31 32 33 34 35 36 37 38 39 4a 31 32 33 34 35 36 37 38 39 4b 31 32 33 34 35 36 37 38 39 4c 31 32 33 34 35 36 37 38 39 4d 31 32 33 34 35 36 37 38 39 4e 31 32 33 34 35 36 37 38 39 4f 31 32 33 34 35 36 37 38 39 50 31 32 33 34 35 36 37 38 39 51 31 32 33 34 35 36 37 38 39 52 31 32 33 34 35 36 37 38 39 53 31 32 33 34 35 36 37 38 39 54 31 32 33 34 35 36 37 38 39 55 31 32 33 34 35 36 37 38 39 56 31 32 33 34 35 36 37 38 39 57 31 32 33 34 35 36 37 38 39 58 31 32 33 34 35 36 37 38 39 59 5a 31 32 33 34 aa 4b')

def SC_prepare(payload, command):
  payload_byte_array = payload
  length = len(payload_byte_array)
  length_bytes = struct.pack('B',length)  

  # Keeping 2 bytes separate for sync characters
  transmission = bytearray.fromhex('48 65')

  # adding 2 bytes for header
  packet=bytearray.fromhex(command) 

  # adding 2 bytes for payload size
  packet.extend(bytearray.fromhex('00'))
  packet.extend(length_bytes)

  # adding 2 bytes for header checksum
  header_checksum = SC_fletcher(packet)
  packet.extend(struct.pack('B', header_checksum[0]))
  packet.extend(struct.pack('B', header_checksum[1]))
  packet.extend(payload_byte_array)

  #adding 2 bytes for payloads checksum
  payload_checksum = SC_fletcher(payload)
  packet.extend(struct.pack('B', payload_checksum[0]))
  packet.extend(struct.pack('B', payload_checksum[1]))

  if debug_flag is True:
      print "Payload:  ", length, " bytes out of a maximum 255\r\n"
      print "bytesize: ", len(packet), " bytes\r\n"
      print "bytesize: ", len(packet), " bytes\r\n"
      print "checksum: ", header_checksum#, " should be: (12,48)" 
      print "bytesize: ", len(packet), " bytes\r\n"
      print "Adding", len(payload_byte_array), "bytes for payload .."
      print "transmit: ", toHex(str(packet))
      print "Message:  ", toHex(str(payload_byte_array))
      print "bytesize:  ", len(payload_byte_array), " bytes\r\n"
      print "checksum: ", payload_checksum#, " should be: (170,75)\r\n"#aa,4b

  transmission.extend(packet)
  print "total payload: ", len(transmission), " bytes\r\n"
  print 'SENDING:: ', toHex(str(transmission))  
  return transmission

def SC_transmit(payload): 
  payload_byte_array = payload.encode('utf-8')
  length = len(payload_byte_array)
  length_bytes = struct.pack('B',length)  
  print "\nPayload:  ", length, " bytes out of a maximum 255"

  #Keeping 2 bytes separate for sync characters
  transmission = bytearray.fromhex('48 65')

  # Adding 2 bytes for header 
  header = '10 03' 
  packet=bytearray.fromhex(header) 

  # adding 2 bytes for payload size
  packet.extend(bytearray.fromhex('00'))
  packet.extend(length_bytes)

  # adding 2 bytes for header checksum
  header_checksum = SC_fletcher(packet)
  packet.extend(struct.pack('B', header_checksum[0]))
  packet.extend(struct.pack('B', header_checksum[1]))

  # adding", len(payload_byte_array), "bytes for payload .."
  packet.extend(payload_byte_array)

  # adding 2 bytes for payload checksum"
  payload_checksum = SC_fletcher(payload)
  packet.extend(struct.pack('B', payload_checksum[0]))
  packet.extend(struct.pack('B', payload_checksum[1]))

  transmission.extend(packet)
  print "\ttotal payload: ", len(transmission), " bytes\n"
  print 'SENDING:: ', toHex(str(transmission))  
  return transmission

def SC_setConfig():
  return bytearray.fromhex('48 65 10 06 00 22 38 74 00 00 01 01 00 00 48 33 02 00 98 93 06 00 56 41 33 4f 52 42 56 45 32 43 55 41 05 00 00 00 41 80 00 00 31 70');

def SC_writeFlash():
  return bytearray.fromhex('48 65 10 08 00 10 28 68 0B 91 F1 D5 4F 93 2D C6 38 2D C6 9F 19 79 00 CF 1A 33')

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

def decToHex(n):
  if n < 16:
    return toHex(n)
  mod = n % 16
  n /= 16
  return DectoHex(n) + str(toHex(mod))

def toStr(s):
  return s and chr(atoi(s[:2], base=16)) + toStr(s[2:]) or ''

def intToBytes(val, num_bytes):
  return [(val & (0xff << pos*8)) >> pos*8 for pos in range(num_bytes)]
