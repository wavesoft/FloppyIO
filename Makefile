CPPFLAGS=-g -O2

test: test.cpp src/disk.o src/errorbase.o
	g++ $(CPPFLAGS) -o test src/disk.o src/errorbase.o test.cpp

