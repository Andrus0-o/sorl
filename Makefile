CXX      ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra
SRC      := $(wildcard src/*.cpp)
OBJ      := $(patsubst src/%.cpp,build/%.o,$(SRC))
TARGET   := build/sorlc

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build:
	mkdir -p build

test: $(TARGET)
	./$(TARGET) examples/example.sr
	./program
	! ./$(TARGET) examples/bad_example.sr
	rm -f program

clean:
	rm -rf build program
