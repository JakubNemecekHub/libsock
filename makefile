.DEFAULT_GOAL := all

CXX = g++

CXXFLAGS_RELEASE = -std=c++23 -O2 -DNDEBUG -s -Wall -c -I ./include/sock
CXXFLAGS_DEBUG = -std=c++23 -O0 -g -Wall -c -I ./include/sock

SOURCE = ./src/main.cpp
TARGET_O = ./build/lib.o
TARGET_A = ./lib/libsock.a

MODE ?= debug

ifeq ($(MODE), release)
	CXXFLAGS = $(CXXFLAGS_RELEASE)
else
	CXXFLAGS = $(CXXFLAGS_DEBUG)
endif

.PHONY: help
help:	## Show this help end exit
	@grep -E '^[a-zA-Z_-]+.*:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {split($$1, a, ":"); printf "\033[36m%-20s\033[0m %s\n", a[1], $$2}'

all: $(TARGET_O) ## Build the object file, default MODE = debug

$(TARGET_O): $(SOURCE)
	mkdir -p ./build
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET_O)

lib: all ## Create library files
	ar rcs $(TARGET_A) $(TARGET_O) 

install: lib ## Install library files
	cp $(TARGET_A) /usr/local/lib
	cp -r include/sock /usr/local/include

clear: ## Clear project's targets
	rm -f $(TARGET_O)
	rm -f $(TARGET_A)
