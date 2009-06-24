CC=gcc
CFLAGS="-Wall"
INSTALLDIR=/usr/bin

all: tweet cltwitter-update-cache

tweet: definitions.h application_helpers.c network_helpers.c oauth_helpers.c string_io_helpers.c xml_helpers.c tweet.c
	$(CC) `xml2-config --cflags` $(CFLAGS) -o tweet -lcurl -loauth -lpcre `xml2-config --libs` oauth_secret.o definitions.h application_helpers.c oauth_helpers.c string_io_helpers.c network_helpers.c xml_helpers.c tweet.c

cltwitter-update-cache: definitions.h application_helpers.c oauth_helpers.c string_io_helpers.c network_helpers.c xml_helpers.c cltwitter-update-cache.c
	$(CC) `xml2-config --cflags` $(CFLAGS) -o cltwitter-update-cache -lcurl -loauth `xml2-config --libs` oauth_secret.o definitions.h application_helpers.c oauth_helpers.c string_io_helpers.c network_helpers.c xml_helpers.c cltwitter-update-cache.c
  
clean: 
	rm -f tweet cltwitter-update-cache

install: install_tweet install_completion

install_tweet:
	cp tweet $(INSTALLDIR)

install_completion:
	cp cltwitter-update-cache $(INSTALLDIR)
	cp completion/completion.bash /etc/bash_completion.d/cltwitter
	@echo
	@echo "=========================================================="
	@echo "Installation done!"
	@echo "=========================================================="
