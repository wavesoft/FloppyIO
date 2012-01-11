
all: errorbase.o disk.o floppyIO.o

clean:
	rm *.o

errorbase.o: errorbase.cpp
	g++ -c -o errorbase.o errorbase.cpp

disk.o: disk.cpp
	g++ -c -o disk.o disk.cpp

floppyIO.o: floppyIO.cpp
	g++ -c -o floppyIO.o floppyIO.cpp
