CC = gcc
CFLAGS = -Wall
PROG = tweet
LIBS = -lcurl -lpcre
SRCS = definitions.h network_helpers.c string_io_helpers.c tweet.c
INSTALLDIR = /usr/bin

$(PROG): $(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean: 
	rm -f $(PROG)

install:
	cp $(PROG) $(INSTALLDIR)
