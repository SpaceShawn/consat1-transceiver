CC=gcc
CXX=g++
CXX_FLAGS=-fpermissive
MICROCC=microblazeel-xilinx-linux-gnu-gcc
MICROPP=microblazeel-xilinx-linux-gnu-g++
#CFLAGS=-static
MICROCFLAGS=-mcpu=v8.40.b -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -mlittle-endian -Wall
DEBUGFLAGS=-ggdb -g -gdwarf-2 -g3 #gdwarf-2 + g3 provides macro info to gdb
MICROINCPATH=-I./inc/Q6/
INCPATH=-I./inc/PC/
LIBPATH=-L./lib

buildBinCpp:
<<<<<<< HEAD
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100.o -ltimer 

buildBin:
	$(CC) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100.o -ltimer 

=======
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100.o -ltimer -lshakespeare
buildBin:
	$(CC) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100.o -ltimer -lshakespeare
>>>>>>> 176a5acad47c36db1ba2d78e2713517086a84821
buildQ6:
	$(MICROCC) $(CSS_FLAGS) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100-mbcc.o -ltimer-mbcc

buildQ6Cpp:
<<<<<<< HEAD
	$(MICROPP) $(CXX_FLAGS) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100-mbcc.o -ltimer-mbcc
=======
	$(MICROPP) $(CXX_FLAGS) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100-mbcc.o -ltimer-mbcc -lshakespeare-mbcc
>>>>>>> 176a5acad47c36db1ba2d78e2713517086a84821
