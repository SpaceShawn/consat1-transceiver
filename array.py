from array import *
from math import *
from transclib import *
import struct

header = bytearray.fromhex('48 65 10 03 00')
data = array('B',[121,233,242])
#for i in data:
print str(data.itemsize)  

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
