CC=gcc
CXX=g++
MICROCC=microblaze-unknown-linux-gnu-gcc
MICROCPP=microblaze-unknown-linux-gnu-g++
CXX_FLAGS=-fpermissive
#CFLAGS=
MICROCFLAGS=-mcpu=v8.10.a -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -ffixed-r31 --sysroot /usr/local/lib/mbgcc/microblaze-unknown-linux-gnu/sys-root
DEBUGFLAGS=-ggdb -g -gdwarf-2 -g3 #gdwarf-2 + g3 provides macro info to gdb
MICROINCPATH=-I./inc/
INCPATH=-I./inc/
LIBPATH=-L./lib

buildBinCpp:
	$(CXX) $(CXX_FLAGS) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100.x -lhe100-cpp -ltimer -lrt
buildBin:
	$(CC) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100.x -lhe100 -ltimer -lrt
buildQ6:
	$(MICROCC) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) $(DEBUGFLAGS) src/test_he100.c -o bin/test_he100Q6.x -lhe100-mbcc -ltimer-mbcc -lrt

#buildAllTests: buildUnitTests
#buildUnitTests:
#	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INCPATH) $(INCTESTPATH) $(LIBPATH) src/*.c tests/unit/*.c -o bin/AllUnitTests $(LIBS)
