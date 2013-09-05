#!/usr/bin/env python
"""Calculate the checksum of telemetry packets.

Each of these methods calculates a fletcher checksum, though each does so
in a different way. The use of one over another is dictated by performance
needs, and the idiosyncracies of each are explained here.

tm_checksum is the recommended method to use if you are unsure of what the
task requires. It is optimized for handling the header and data of packets
separately (to avoid the expensive operation of concatenating the header)
and packet.

fletcher_checksum can compute the checksum of anything. It is slightly
faster than tm_checksum if header and data are already concatenated. Tests
have shown that it might still be faster than tm_checksum if you use a
slice or concatenation operation, but it varies from system to system. The
difference is too slight to change existing code for its sake.

old_checksum is retained for testing/benchmark purposes. Bizarrely enough,
when using Pypy (a faster implementation of python with a JIT compiler),
tests indicated that this method is -dramatically- faster than the others,
taking less than 10% of the time tm_checksum does. Using standard Python,
however, it seems take ~50% more time to compute, so it is strongly
recommended for the sake of portability that one does not use this in real
code.

Author: Matthew Lefavor (based on code originally by Eric Raaen)
Date:   2011-07-13

"""
# For 2.5 compatibility
from __future__ import absolute_import
from __future__ import with_statement

from itertools import islice, izip

__all__ = ['tm_checksum', 'fletcher_checksum', 'old_checksum']

def tm_checksum(header, data):
    """Calculate the checksum of a packet.
    
    For any two strings header and data, tm_checksum(header, data) is
    equivalent to fletcher_checksum(header + data). This function is
    provided for optimization purposes to avoid the overhead incurred
    by python's inefficient string concatenation.
    
    Parameters:
    header  An eight-byte bitstring representing the header of a telemetry
            packet. If header is not exactly eight bytes long, the function
            will raise a ValueError.
    data    A variable length bitstring containing the rest of the raw data
            contained in the telemetry packet (not including the checksum
            bytes).
    
    """
    if len(header) != 8:
        raise ValueError('header must have exactly 8 bytes')
    
    # We know how long the header is, and a loop means overhead
    sum1 = (ord(header[0]) << 8) | ord(header[1])
    sum2 = sum1
    sum1 = sum1 + ((ord(header[2]) << 8) | ord(header[3]))
    sum2 = sum2 + sum1
    sum1 = sum1 + ((ord(header[4]) << 8) | ord(header[5]))
    sum2 = sum2 + sum1
    sum1 = sum1 + ((ord(header[6]) << 8) | ord(header[7]))
    sum2 = sum2 + sum1
    
    # byte1, byte2 are two consecutive bytes in data.
    # e.g., if data were 'abcdefghijk', we'd have
    # 'ab', 'cd', 'ef', 'gh', 'ij' (k skipped)
    # Even with generator overhead, this is much, much faster than indexing
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
    
    return (sum2 << 16) | sum1

def fletcher_checksum(data):
    """A general-purpose fletcher's checksum algorithm.
    
    Performance is comparable to tm_checksum.
    
    """
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
    
    return (sum2 << 16) | sum1

def old_checksum (header, data):
    """The old checksumming algorithm.
    
    See the parameters for tm_checksum for more information, as well as
    the module docstring. Summary: For most users, this method is nearly
    twice as slow as the others.
    
    """
    sum1 = 0xFFFF
    sum2 = 0xFFFF
    
    for i in range(0, len(header) / 2):
        word = (ord(header[i*2+0])<<8) | ord(header[i*2+1])
        sum1 += word
        sum2 += sum1
    
    for i in range(0, len(data)/2):
        word = (ord(data[i*2+0])<<8) | ord(data[i*2+1])
        sum1 += word
        sum2 += sum1
    
    if len(data) % 2 == 1:
        word = ord(data[-1])<<8
        sum1 += word
        sum2 += sum1
    
    while sum1 > 0xFFFF:
        sum1 = (sum1 & 0xFFFF) + (sum1 >> 16)
    while sum2 > 0xFFFF:
        sum2 = (sum2 & 0xFFFF) + (sum2 >> 16)
    
    return (sum2<<16) | sum1
