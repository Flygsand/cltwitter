CC = gcc
CFLAGS = -Wall
PROG = tweet
LIBS = -lcurl
SRCS = tweet.c
INSTALLDIR = /usr/bin

$(PROG): $(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean: 
	rm -f $(PROG)

install:
	cp $(PROG) $(INSTALLDIR)
