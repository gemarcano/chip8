CXXFLAGS=-O0 -Wall -Wextra -pedantic -g3 -std=c++14

LDLIBS=-lSDL2

chip8-emulator: chip8.o main.o
	$(CXX) $^ -o $@ $(LDLIBS)

main.o: main.cpp chip8.h

chip8.o: chip8.cpp chip8.h

clean:
	rm -f chip8.o main.o chip8-emulator
