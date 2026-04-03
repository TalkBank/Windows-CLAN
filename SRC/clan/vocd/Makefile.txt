
CC = gcc
#CC = /usr/gnu/gcc

#CFLAGS += -g

CFLAGS += -Wall

LIBS = -lm

OBJS = args.o speaker.o tokens.o filter.o dcompute.o main.o 

vocd:   $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f core vocd *.o
