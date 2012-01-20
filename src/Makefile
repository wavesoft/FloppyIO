CPPFLAGS=-O2

all: errorbase.o floppyIO.o flpdisk.o

clean:
	rm *.o

errorbase.o: errorbase.cpp
	g++ $(CPPFLAGS) -c -o errorbase.o errorbase.cpp

disk.o: disk.cpp
	g++ $(CPPFLAGS) -c -o disk.o disk.cpp

flpdisk.o: flpdisk.cpp
	g++ $(CPPFLAGS) -c -o flpdisk.o flpdisk.cpp

floppyIO.o: floppyIO.cpp
	g++ $(CPPFLAGS) -c -o floppyIO.o floppyIO.cpp
