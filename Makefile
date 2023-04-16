# Sample Makefile to compile C programs

CC = gcc
CFLAGS = -Wall -g #replace -g with -O when not debugging

DEPS	= queue.h Makefile
OBJS	= queue.o cron.o

EXECS	= queuetest

all:	$(EXECS)

%.o:	%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

queuetest:	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:	
	/bin/rm -i *.o $(EXECS)
