$(IOLIB_OBJ) : %.o: %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CCFLAGS) $< -o $@
.c.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CCFLAGS) $< -o $@
.cpp.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CCFLAGS) $< -o $@
.s.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CCFLAGS) $< -o $@
.cxx.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CCFLAGS) $< -o $@
%.o : %.cxx
	$(CC) -c $(DEFINES) $(INCLUDES) $(CCFLAGS) $< -o $@

clean:
	rm -rf $(IOLIB_OBJ) $(EXES) $(OBJS) $(CLEAN)
	
