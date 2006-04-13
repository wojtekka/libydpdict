# $Id$

CC = gcc
CFLAGS = -Wall -pedantic -O3

all:	gentable ydptest ydpstress

ydptest:	ydptest.c
	$(CC) $(CFLAGS) $< -o $@ -lydpdict

ydpstress:	ydpstress.c
	$(CC) $(CFLAGS) $< -o $@ -lydpdict
	
clean:
	rm -f gentable ydptest ydpstress *.o *.html *~ core

.PHONY:	all clean

