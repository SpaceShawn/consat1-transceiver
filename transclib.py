from math import *
import struct
import sys

def SC_computeFletcher(data, size, modulo, limit=None):
	valA, valB = 1, 0
	length = len(data)	
	if isinstance(data, str):
		if limit is not None and length > limit:
			data = data[:limit]
		for char in data:
			valA += ord(char)
			valB += valA
			valA %= modulo
			valB %= modulo
	else:
		if limit is not None and length > limit:
			data = data[:limit]
		for c in data:
			valA += c
			valB += valA
			valA %= modulo
			valB %= modulo
#	return (valA << (size/2), valB)
	return (valA, valB)
#	return (valB << (size/2)) + valA

def SC_fletcher8(data):
  return SC_computeFletcher(data, 8, 255, limit=21)

def SC_fletcher32(data):
  return SC_computeFletcher(data, 32, 65535, limit=360)

def SC_fletcher(data):
  return SC_fletcher8(data)

def SC_noop(): 
  return bytearray.fromhex('48 65 10 01 00 00 11 43 00 00')

def SC_transmit(payload):
  payload = '12345678'
  print '\r\n>> Hardcoded payload: "12345678"'
  payload_byte_array = payload.encode('utf-8')
  length = len(payload_byte_array)
  length_bytes = struct.pack('B',length)  
  print "Payload:  ", length, " bytes out of a maximum 255\r\n"

  print 'Adding 4 bytes for header ...'
  header = '48 65 10 03' 
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
  print "checksum: ", header_checksum
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  print "Adding", len(payload_byte_array), "bytes for payload .."
  packet.extend(payload_byte_array)
  print "Message:  ", toHex(str(payload_byte_array))
  print "Message:  ", len(payload_byte_array), " bytes"
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  print "Adding 2 bytes for payload checksum"
  payload_checksum = SC_fletcher(payload)
  packet.extend(struct.pack('B', payload_checksum[0]))
  packet.extend(struct.pack('B', payload_checksum[1]))
  print "checksum: ", payload_checksum
  print "transmit: ", toHex(str(packet))
  print "bytesize: ", len(packet), " bytes\r\n"

  print 'SENDING:: ', toHex(str(packet))  
  return packet

def SC_getConfig():
  return ''

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
