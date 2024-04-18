TARGET=app
CXX=g++
CXXFLAGS=-g -std=c++17
HPATH=-I./
LIBS=-lpthread

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) -o $@ $^ $(HPAH) $(CXXFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -rf $(TARGET)
