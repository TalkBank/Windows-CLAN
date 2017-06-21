CFLAGS = -O2 -DNDEBUG
#CFLAGS = -g
OBJS = maxent.o blmvm.o

megrasp: $(OBJS) ksutil.h megrasp.cpp 
	g++ $(CFLAGS) $(OBJS) megrasp.cpp -o megrasp

clean:
	/bin/rm -r -f $(OBJS) *.o *~ megrasp
.cpp.o:
	g++ -c $(CFLAGS) $<
