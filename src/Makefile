CPPFLAGS=-O2

all: errorbase.o disk.o floppyIO.o

clean:
	rm *.o

errorbase.o: errorbase.cpp
	g++ $(CPPFLAGS) -c -o errorbase.o errorbase.cpp

disk.o: disk.cpp
	g++ $(CPPFLAGS) -c -o disk.o disk.cpp

floppyIO.o: floppyIO.cpp
	g++ $(CPPFLAGS) -c -o floppyIO.o floppyIO.cpp
