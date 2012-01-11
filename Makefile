
test: test.cpp src/disk.o src/errorbase.o
	g++ -o test src/disk.o src/errorbase.o test.cpp

