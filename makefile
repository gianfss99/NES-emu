CC = g++
CFLAGS = -std=c++11

all: main

memory.o: memory.cpp memory.hpp
	g++ -std=c++11 -c -g memory.cpp

cpu.o: cpu.cpp memory.hpp cpu.hpp
	g++ -std=c++11 -c -g cpu.cpp

helper.o: helper.cpp helper.hpp
	g++ -std=c++11 -c -g helper.cpp

system.o: system.cpp cpu.hpp helper.hpp system.hpp
	g++ -std=c++11 -c -g system.cpp

main.o: main.cpp memory.hpp cpu.hpp helper.hpp system.hpp
	g++ -std=c++11 -c -g main.cpp

main: main.o memory.o cpu.o helper.o system.o
	g++ -std=c++11 -o main main.o memory.o cpu.o helper.o system.o

clean:
	rm -rf *.o main