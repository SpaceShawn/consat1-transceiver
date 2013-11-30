#!/bin/bash
make buildBinCpp
ar rcs lib/libhe100-cpp.a lib/he100.o
