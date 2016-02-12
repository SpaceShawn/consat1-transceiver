CC=gcc
CXX=g++
CXX_FLAGS=-fpermissive
MICROCC=microblazeel-xilinx-linux-gnu-gcc
MICROPP=microblazeel-xilinx-linux-gnu-g++
BEAGLECC=arm-linux-gnueabi-g++
#CFLAGS=-static
MICROCFLAGS=-mcpu=v8.40.b -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -mlittle-endian -Wall

DEBUGFLAGS=-pedantic -Wextra -Werror -ggdb -g -gdwarf-2 -g3 #gdwarf-2 + g3 provides macro info to gdb
ENV_FLAGS = -DCS1_DEBUG # to enable debug messages

CS1_DIR=../../
SPACE_LIB=$(CS1_DIR)space-lib/
UTLS_DIR=$(SPACE_LIB)utls/
SPACE_TIMER_LIB=$(CS1_DIR)/space-timer-lib/

INCPATH=-I./inc/ -I$(UTLS_DIR)/include -I$(SPACE_LIB)/checksum/inc/ -I$(SPACE_LIB)/shakespeare/inc/ -I$(SPACE_TIMER_LIB)/inc/ -I$(SPACE_LIB)/include/
MICROINCPATH=-I./inc/Q6/
PCINCPATH=-I./inc/PC/
BBINCPATH=-I./inc/BB/

LIBPATHS=-L../../space-lib/shakespeare/lib -L../../space-timer-lib -L./lib

PC_LIBRARIES=-ltimer -lshakespeare -lfletcher -lcrypto -lssl
Q6_LIBRARIES=-ltimer-mbcc -lshakespeare-mbcc -lfletcher-mbcc -lcrypto -lssl

buildBin: mkdirs buildBinDep 
	ar rcs lib/libhe100.a lib/SC_he100-translations.o lib/he100.o lib/SC_serial.o

buildQ6: mkdirs buildQ6Dep 
	ar rcs lib/libhe100-mbcc.a lib/SC_he100-translations.o lib/he100-mbcc.o lib/SC_serialQ6.o

buildBB: mkdirs buildBBDep 
	ar rcs lib/libhe100-BB.a lib/SC_he100-translations.o lib/he100-BB.o lib/SC_serialBB.o

mkdirs: 
	mkdir -p $(CS1_DIR)/HE100-lib/C/lib

# PC architecture

lib/SC_he100-translations.o:
	$(CXX) $(CXX_FLAGS) $(DEBUGFLAGS) -static -c src/SC_he100-translations.c -o lib/SC_he100-translations.o 

lib/SC_serial.o:
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(PCINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c $(UTLS_DIR)src/SC_serial.c -o lib/SC_serial.o  $(PC_LIBRARIES) $(ENV_FLAGS)

buildBinDep: lib/SC_he100-translations.o lib/SC_serial.o
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(PCINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100.o  $(PC_LIBRARIES) $(ENV_FLAGS)

# Q6 architecture

lib/SC_he100-translationsQ6.o:
	$(MICROPP) $(CXX_FLAGS) $(DEBUGFLAGS) -static -c src/SC_he100-translations.c -o lib/SC_he100-translationsQ6.o 

buildQ6Dep: lib/SC_he100-translations.o lib/SC_serialQ6.o
	$(MICROPP) $(CXX_FLAGS) $(MICROCFLAGS) $(INCPATH) $(MICROINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100-mbcc.o $(Q6_LIBRARIES) $(ENV_FLAGS)

lib/SC_serialQ6.o:
	$(MICROPP) $(CXX_FLAGS) $(INCPATH) $(PCINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c $(UTLS_DIR)src/SC_serial.c -o lib/SC_serialQ6.o  $(Q6_LIBRARIES) $(ENV_FLAGS)

# BB architecture

buildBBDep: lib/SC_he100-translations.o lib/SC_serial.o
	$(BEAGLECC) $(CXX_FLAGS) $(INCPATH) $(BBINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c $(UTLS_DIR)src/SC_serial.c -o lib/he100-BB.o -ltimer-BB -lshakespeare-BB -lfletcher-BB $(ENV_FLAGS)

lib/SC_he100-translationsBB.o:
	$(BEAGLECC) $(CXX_FLAGS) $(DEBUGFLAGS) -static -c src/SC_he100-translations.c -o lib/SC_he100-translationsBB.o 

lib/SC_serialBB.o:
	$(BEAGLECC)$(CXX_FLAGS) $(INCPATH) $(PCINCPATH) $(LIBPATHS) $(DEBUGFLAGS) -static -c $(UTLS_DIR)src/SC_serial.c -o lib/SC_serialBB.o  $(BB_LIBRARIES) $(ENV_FLAGS)

clean:
	rm -f lib/*
