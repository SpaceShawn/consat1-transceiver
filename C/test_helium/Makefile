CC=gcc
CXX=g++
MICROCC=microblazeel-xilinx-linux-gnu-gcc
MICROPP=microblazeel-xilinx-linux-gnu-g++
CXX_FLAGS=-fpermissive
#CFLAGS=
MICROCFLAGS=-mcpu=v8.40.b -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -mlittle-endian -Wall
DEBUGFLAGS=-ggdb -g -gdwarf-2 -g3 #gdwarf-2 + g3 provides macro info to gdb
MICROINCPATH=-I./inc/
INCPATH=-I./inc/
LIBPATH=-L./lib

buildBinCpp:
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100.x -lhe100-cpp -ltimer -lrt
buildBin:
	$(CC) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100.x -lhe100 -ltimer -lrt
buildQ6:
	$(MICROCC) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100Q6.x -lnamedpipe-mbcc -lhe100-mbcc -ltimer-mbcc -lrt
buildQ6Cpp:
	$(MICROPP) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100Q6.x -lnamedpipe-mbcc -lhe100-mbcc -ltimer-mbcc -lrt
buildDummy: 
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100.x -ltimer -lrt

#buildAllTests: buildUnitTests
#buildUnitTests:
#	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INCPATH) $(INCTESTPATH) $(LIBPATH) src/*.c tests/unit/*.c -o bin/AllUnitTests $(LIBS)
