TARGET = stool
LIBS = -lboost_program_options

CXX = g++
CXXFLAGS = --std=c++2a -Wall -O3

.PHONY: default debug all clean

default: $(TARGET)
all: default


debug: CXXFLAGS += -DDEBUG -g
debug: all

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.hpp)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
