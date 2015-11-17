#Makefile

TOP_DIR = ../..

include ../../mk/global_def.mk

OBJS = test_threadpool.o

EXES = test_thread

CLEAN += *.o *.app

all : $(EXES)

test_thread : $(IOLIB_OBJ) $(OBJS)
		 $(LD) $(INCLUDES) $(LDFLAGS) -L$(TPLIB_DIR) $(IOLIB_OBJ) $(OBJS) -o test_threadpool.app


include ../../mk/global_rule.mk
