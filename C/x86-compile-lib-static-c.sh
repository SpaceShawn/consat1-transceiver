#!/bin/bash
make buildBin
ar rcs lib/libhe100-c.a lib/he100.o
