CXX = g++
CXXFLAGS = -std=c++11 -g
SRCS := $(shell ls src/*.cpp)
HEADERS := $(shell ls inc/*.hpp)
OBJS := $(SRCS:%.cpp=%.o)
TARGETS := main

.PHONY: all clean

all: $(TARGETS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

main: $(OBJS)
	$(CXX) -o main $(OBJS)

clean:
	rm -rf *.o src/*.o $(TARGETS)