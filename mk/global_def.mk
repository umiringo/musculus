# TOP_DIR = ..
IOLIB_DIR = $(TOP_DIR)/iolib
UTIL_DIR = $(IOLIB_DIR)/util
TP_DIR = $(TOP_DIR)/thirdparty
TPLIB_DIR = $(TP_DIR)/lib
CC = g++ 
LD = g++

INCLUDES = -I. -I$(TOP_DIR) -I$(IOLIB_DIR) -I$(TP_DIR)/include

IOLIB_SRC = $(UTIL_DIR)/timer.cpp $(UTIL_DIR)/itimer.cpp $(UTIL_DIR)/threadpool.cpp

DEFINES = -D_GNU_SOURCE
CCFLAGS = -Wall -g -ggdb -std=c++11
LDFLAGS = -g -ggdb -pthread -std=c++11

IOLIB_OBJ := $(IOLIB_SRC:%.cpp=%.o) 
