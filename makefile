CXX = g++
CXXFLAGS = -std=c++11 -g
SRCS := $(shell ls src/*.cpp)
HEADERS := $(shell ls inc/*.hpp)
OBJS := $(SRCS:%.cpp=%.o)
TARGETS := main

.PHONY: all clean

all: $(TARGETS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) `sdl2-config --cflags --libs`

main: $(OBJS)
	$(CXX) -o main $(OBJS) `sdl2-config --cflags --libs`

clean:
	rm -rf *.o src/*.o $(TARGETS)