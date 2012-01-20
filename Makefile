CPPFLAGS=-g -O2

test: write.cpp src/flpdisk.o src/errorbase.o src/floppyIO.o
	g++ $(CPPFLAGS) -o test src/flpdisk.o src/errorbase.o src/floppyIO.o test.cpp

