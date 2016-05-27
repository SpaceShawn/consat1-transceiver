from math import *
import struct
import sys
import time
import binascii # to convert incoming messages to ascii
from itertools import *
from functools import reduce
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

def SC_digipeat(ser):
  
  return bytearray.fromhex('48 65 10 01 00 00 11 43 00 00')

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

def SC_listenLoop(ser):
    while True:
        SC_listen(ser)

def SC_listen(ser):
    '''
    action = raw_input(": ")
    if action == 'q':
      print("Stopping listener")
      break
    '''
    out = ''
    while ser.inWaiting() > 2:
      # read 6 bytes to capture the 'length' byte
      out += ser.read(6)
    if out != '':
      # store the header bytes in an array
      header = bytearray.fromhex(toHex(out))
      '''
      print "Header: " 
      for h in header: print hex(h),
      print "End Header"
      '''

      # extract and print the payload length
      payload_length = int(header[5])
      '''
      print "Detected payload length" , hex(header[5]), str(payload_length), "\n\r"
      '''

      # wait a 1/4 sec
      time.sleep(0.25)

      # get the rest of the data
      try:
        out += ser.read(payload_length+5)
      except BufferError:
        print "Incomplete buffer: ", toHex(out), "\n\r"
      '''
      print "out:", toHex(out), "\n\r"
      '''

      # calculate frame length
      expected_frame_length = payload_length + 10
      
      # dump the raw data into a bytearray representing the frame
      frame = bytearray.fromhex(toHex(out[0:expected_frame_length]))
      frame_length = len(frame)
      '''
      print "Frame: "
      for fb in frame: print hex(fb),
      print "End Frame"
      '''

      # compare expected and actual frame lengths
      '''
      if frame_length != expected_frame_length:
          print "WARNING: mismatched frame_length. Expected:",expected_frame_length, "Actual:",frame_length
      else: 
          print "SUCCESS: frame_length matches expected_frame_length! Expected:",expected_frame_length, "Actual:",frame_length 
      '''
      # determine frame segment lengths
      header_length = 8
      header_checksum_pos = 6
      preamble_length = 16
      first_element = header_length+preamble_length
      real_payload_length = payload_length-preamble_length
      '''
      print "first element:" + str(first_element)
      print "real payload length:" + str(real_payload_length)
      '''

      # extract the header checksum bytes
      header_checksum = (frame[header_checksum_pos:header_checksum_pos+1])
      '''
      print "Header Checksum:"
      for hcb in header_checksum: print hex(hcb),
      print "\nEnd Header Checksum:"
      '''

      # extract the payload bytes
      payload = (frame[header_length:first_element+real_payload_length-1])
      '''
      print "Payload:"
      for pyb in payload: print hex(pyb),
      print "\nEnd payload"
      '''

      # extract the payload checksum bytes
      payload_checksum = (frame[frame_length-2:frame_length-1])
      '''
      print "Payload checksum:"
      for pyb in payload: print hex(pyb),
      print "\nEnd payload checksum"
      '''

      # check for kenwood garbage
      if payload[0] == 0x86 and payload[1] == 0xa2 and payload[2] == 0x40:
        preamble_start = header_length
        preamble_end = header_length+preamble_length-1
        postamble_start = first_element+real_payload_length-3
        postamble_end = first_element+real_payload_length-2
        real_payload_start = first_element
        real_payload_end = first_element+real_payload_length-1-2

        # extract the preamble bytes
        preamble = (frame[preamble_start:preamble_end])
        '''
        print "Preamble:"
        for pb in preamble: print hex(pb),
        print "\nEnd preamble:"
        '''

        # extract the preamble bytes
        postamble = (frame[postamble_start:postamble_end])
        '''
        print "Postamble"
        for pob in postamble: print hex(pob),
        print "\nEnd postamble:"
        '''

        # extract the payload bytes
        payload = (frame[real_payload_start:real_payload_end])
        '''
        print "Real payload:"
        for pyb in payload: print hex(pyb),
        print "\nEnd real payload"
        '''

      #payload = payload.strip()
      #payload = payload.strip().decode('hex')
      #payload = payload.decode('hex')
      #payload = binascii.a2b_hex(payload.strip())

      '''
      print "\n<< Incoming:\n", toHex(out), "\n", toHex(out.strip()).decode('hex'), "\n"
      print "Sync Bytes: ", chr(data[0]), chr(data[1]), "\r"
      print "Command Type: ", str(hex(data[2])), str(hex(data[3])), "\r"
      print "Payload Size: ", data[4], str(int(data[5])), "\r"
      print "Header Check: ", data[6], data[7], "\r"
      print "Payload Check: ", data[len(data)-2], data[len(data)-1], "\r\n\r\n"
      print "Payload: ", str(payload),"\r\n"
      '''
      
      return binascii.b2a_qp(payload)

def SC_testTransmit():
## weirdness: "A payload checksum is then used to verify the accuracy of the payload. The checksum is calculated across all pertinent bytes of the message excluding the two sync characters of each message 'He'
## possible interpretation:
### header checksum is of header minus sync bytes, payload checksum is of total minus sync bytes
### header checksum is of header minus sync bytes, payload checksum is of payload minus sync bytes
### header checksum is of header in full, payload checksum is of total minus sync bytes
###  payload = '12345678'
##  print("\r\n>> Hardcoded payload: "12345678"'  return bytearray.fromhex('48 65 10 03 0a 1d 53 64 65 66 67 68 69 6a 6b 6c 6d 15 21')
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
  print("total payload: ", len(transmission), " bytes\r\n")
  print("Sending: ", toHex(str(transmission)))
  return transmission

def SC_transmit(payload):
  payload_byte_array = payload.encode('utf-8')
  length = len(payload_byte_array)
  length_bytes = struct.pack('B',length)  
  '''
  print "\nPayload:  ", length, " bytes out of a maximum 255"
  '''

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
  '''
  print "\ttotal payload: ", len(transmission), " bytes\n"
  '''
  print 'Sending::', toHex(str(transmission))  

  return transmission

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
