CPPFLAGS=-g -O2

test: test.cpp src/flpdisk.o src/errorbase.o src/floppyIO.o
	g++ $(CPPFLAGS) -o test src/flpdisk.o src/errorbase.o src/floppyIO.o test.cpp

read: read.cpp src/flpdisk.o src/errorbase.o src/floppyIO.o
	g++ $(CPPFLAGS) -o read src/flpdisk.o src/errorbase.o src/floppyIO.o read.cpp

write: write.cpp src/flpdisk.o src/errorbase.o src/floppyIO.o
	g++ $(CPPFLAGS) -o write src/flpdisk.o src/errorbase.o src/floppyIO.o write.cpp


