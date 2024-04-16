TARGET=app
CXX=g++
CXXFLAGS=-g -lpthread -std=c++11
HPATH=-I./

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS)

.PHONY: clean
clean:
	rm -rf $(TARGET)
