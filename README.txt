Run the mbcc-compile-lib-static-cpp.sh to build the static library which you can build against in your own code.

The Helium 100 is a board made by AstroDev (copyright, blah blah). Our current board is hardwired to operate at 441000 kHz TX and 143000 kHz Rx (TODO fact check)

The Helium 100 Frame looks like this: [2 header bytes] [2 command bytes] [2 length bytes] [2 header checksum bytes] [X payload bytes] [2 payload checksum bytes]

Fletcher's Checksum is used for data integrity checking on the board, as well as in our code. 

The header checksum is calculated against ...
The payload checksum is calculated against all bytes except the first two header bytes, and is stored at the end of the byte sequence as illustrated above.
