CC=gcc
MICROCC=microblaze-unknown-linux-gnu-gcc
#CFLAGS=
MICROCFLAGS=-mcpu=v8.10.a -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -ffixed-r31 --sysroot /usr/local/lib/mbgcc/microblaze-unknown-linux-gnu/sys-root
DEBUGFLAGS=-ggdb -g -gdwarf-2 -g3 #gdwarf-2 + g3 provides macro info to gdb
MICROINCPATH=-I./inc/Q6/
INCPATH=-I./inc/PC/
LIBPATH=-L./lib/

buildBin:
#	$(CC) $(CFLAGS) $(INCPATH) $(LIBPATH) src/he100.c -o bin/he100.x
	$(CC) $(INCPATH) $(LIBPATH) src/he100.c -o bin/he100.x
buildQ6:
	$(MICROCC) $(MICROCFLAGS) $(MICROINCPATH) $(LIBPATH) src/he100.c -o bin/he100Q6.x 

#buildAllTests: buildUnitTests
#buildUnitTests:
#	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INCPATH) $(INCTESTPATH) $(LIBPATH) src/*.c tests/unit/*.c -o bin/AllUnitTests $(LIBS)
