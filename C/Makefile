CC=gcc
CXX=g++
CXX_FLAGS=-fpermissive
MICROCC=microblazeel-xilinx-linux-gnu-gcc
MICROPP=microblazeel-xilinx-linux-gnu-g++
BEAGLECC=arm-linux-gnueabi-g++
#CFLAGS=-static
MICROCFLAGS=-mcpu=v8.40.b -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -mlittle-endian -Wall
DEBUGFLAGS=-ggdb -g -gdwarf-2 -g3 #gdwarf-2 + g3 provides macro info to gdb

SPACE_LIB=../../space-lib
SPACE_TIMER_LIB=../../space-timer

INCPATH=-I./inc/ -I$(SPACE_LIB)/checksum/inc -I$(SPACE_TIMER_LIB)/inc -I$(SPACE_LIB)/include
MICROINCPATH=-I./inc/Q6/
PCINCPATH=-I./inc/PC/
BBINCPATH=-I./inc/BB/

LIBPATHS=-L../../space-lib/shakespeare/lib -L../../space-timer-lib -L./lib

PC_LIBRARIES=-ltimer -lshakespeare -lfletcher -lcrypto -lssl
Q6_LIBRARIES=-ltimer-mbcc -lshakespeare-mbcc -lfletcher-mbcc -lcrypto -lssl

buildBin:
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(PCINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100.o  $(PC_LIBRARIES)

buildQ6:
	$(MICROPP) $(CXX_FLAGS) $(MICROCFLAGS) $(INCPATH) $(MICROINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100-mbcc.o $(Q6_LIBRARIES)

buildBB:
	$(BEAGLECC) $(CXX_FLAGS) $(INCPATH) $(BBINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100-BB.o -ltimer-BB -lshakespeare-BB -lfletcher-BB

clean:
	rm -f lib/*
