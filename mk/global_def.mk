# TOP_DIR = ..
IOLIB_DIR = $(TOP_DIR)/iolib
UTIL_DIR = $(IOLIB_DIR)/util
TP_DIR = $(TOP_CDIR)/thirdparty
JSON_DIR = $(TP_DIR)/json/include

CC = g++ 
LD = g++

INCLUDES = -I. -I$(TOP_SRCDIR) -I$(IOLIB_DIR) -I$(JSON_DIR)

IOLIB_SRC = $(UTIL_DIR)/timer.cpp $(UTIL_DIR)/itimer.cpp $(UTIL_DIR)/threadpool.cpp

DEFINES = -D_GNU_SOURCE -pthread 
CCFLAGS = -Wall -c -g -ggdb -std=c++11
LDFLAGS = -g -ggdb -pthread -std=c++11
IOLIB_OBJ := $(IOLIB_SRC:%.cpp=%.o)


