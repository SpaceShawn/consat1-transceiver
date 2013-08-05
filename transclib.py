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

def SC_transmitHard():
  message = '12345678'
  header = '48 65 10 03' 
  packet=bytearray.fromhex(header) 

  packet.extend(bytearray.fromhex('00'))
  #length = unsigned(int(len(message))) # % 2**8 

  msg_byte_array = message.encode('utf-8')
  length = len(msg_byte_array)
  lengthbytes = struct.pack('B',length)
  packet.extend(lengthbytes)

  headerchk = SC_fletcher(packet)
  packet.extend(struct.pack('B', headerchk[0]))
  packet.extend(struct.pack('B', headerchk[1]))

  print 'header: ', toHex(str(packet))

  packet.extend(msg_byte_array)

  msgchecksum = SC_fletcher(message)
  packet.extend(struct.pack('B', msgchecksum[0]))
  packet.extend(struct.pack('B', msgchecksum[1]))

  print 'full data: ', toHex(str(packet))  

def SC_transmit(msg): 
	length = unsigned(int(len(msg)))  % 2**8
	#print "var length: " + type(length)
	lengthbytes = struct.pack('B', length)
	print 'payload leng: ' + str(length)
	msg_checksum = SC_fletcher(msg)
	print 'messagecheck: ' + str(msg_checksum)
	
	# header
	header = '48 65 10 03' #H E transmit ?? 1st_length
	packet = bytearray.fromhex(header)
	print 'header base : ' + toHex(str(packet))

	# length
	# add '00' byte before length
	packet.extend(bytearray.fromhex('00'))
	packet.extend(lengthbytes)
	print 'added length: ' + toHex(str(packet))
	print 'length.size : ' + SC_size(lengthbytes) + ' bytes'
	header_checksum = SC_fletcher(packet)
	print 'headck1 size: ' + SC_size(header_checksum[0]) + " bytes";
	print 'headck2 size: ' + SC_size(header_checksum[1]) + " bytes";
	print 'header check: ' + str(header_checksum)

	packet.extend(struct.pack('B', header_checksum[0]))
	packet.extend(struct.pack('B', header_checksum[1]))
#	packet.extend(intToBytes(header_checksum[0],1))
#	packet.extend(intToBytes(header_checksum[1],1))
		
	print 'full header : ' + toHex(str(packet))
	
	packet.extend(msg.encode('utf-8'))

	print 'fullhead+msg: ' + toHex(str(packet))
	print 'msg checksum: ' + toHex(str(msg_checksum[0])) + toHex(str(msg_checksum[1]));
	print 'msgchk1 size: ' + SC_size(msg_checksum[0]) + " bytes";
	print 'msgchk2 size: ' + SC_size(msg_checksum[1]) + " bytes";
	
	packet.extend(struct.pack('B', msg_checksum[0]))
	packet.extend(struct.pack('B', msg_checksum[1]))
#	packet.extend(intToBytes(msg_checksum[0],1))
#	packet.extend(intToBytes(msg_checksum[1],1))

	print 'fullmsginhex: ' + toHex(str(packet))

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
