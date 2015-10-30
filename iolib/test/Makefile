#Makefile

TOP_SRCDIR = ..
UTIL_DIR = $(TOP_SRCDIR)/util
LD = g++
DEFINES = -Wall -D_GNU_SOURCE
LDFLAGS = -pthread
INCLUDES += -I$(UTIL_DIR)
OBJS = $(UTIL_DIR)/threadpool.o $(UTIL_DIR)/itimer.o $(UTIL_DIR)/timer.o

EXES = thread

CLEAN += *.o *.app

all : $(EXES)

thread : $(OBJS) test_threadpool.o
		 $(LD) $(INCLUDES) $(LDFLAGS) $(OBJS) test_threadpool.o -o test_threadpool.app

clean :
		rm -rf $(CLEAN)
		rm -rf $(UTIL_DIR)/$(CLEAN)
