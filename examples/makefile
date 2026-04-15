.DEFAULT_GOAL := all

CXX = g++

CXXFLAGS = -std=c++23 -O0 -g -Wall -I ../include
CXXLIBS = -L ../lib -l sock

SOURCE_1 = send_udp.cpp
TARGET_1 = send_udp

all: $(TARGET_1)

$(TARGET_1): $(SOURCE_1)
	$(CXX) $(CXXFLAGS) $(SOURCE_1) -o $(TARGET_1) $(CXXLIBS)

clear:
	rm -f $(TARGET_1)