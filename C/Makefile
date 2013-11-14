CC=gcc
MICROCC=microblaze-unknown-linux-gnu-gcc
#CFLAGS=-static
MICROCFLAGS=-c -mcpu=v8.10.a -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -ffixed-r31 --sysroot /usr/local/lib/mbgcc/microblaze-unknown-linux-gnu/sys-root 
DEBUGFLAGS=-ggdb -g -gdwarf-2 -g3 #gdwarf-2 + g3 provides macro info to gdb
MICROINCPATH=-I./inc/Q6/
INCPATH=-I./inc/PC/
LIBPATH=-L./lib

buildBin:
#	 $(CC) $(CFLAGS) $(INCPATH) $(LIBPATH) -lshakespeare src/he100.c -o bin/he100.x
	$(CC) $(INCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100.o -ltimer 
buildQ6:
#	$(MICROCC) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) -lshakespeare-mbcc src/he100.c -o bin/he100Q6.x 
	$(MICROCC) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) $(DEBUGFLAGS) -static -c src/SC_he100.c -o lib/he100-mbcc.o -ltimer-mbcc

#buildAllTests: buildUnitTests
#buildUnitTests:
#	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INCPATH) $(INCTESTPATH) $(LIBPATH) src/*.c tests/unit/*.c -o bin/AllUnitTests $(LIBS)
