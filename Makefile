CPPFLAGS=-g -O2

test: test.cpp src/disk.o src/errorbase.o
	g++ $(CPPFLAGS) -o test src/flpdisk.o src/errorbase.o src/floppyIO.o test.cpp

