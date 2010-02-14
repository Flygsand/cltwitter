bin_PROGRAMS = tweet cltwitter-update-cache

tweet_SOURCES = definitions.h application_helpers.c network_helpers.c oauth_helpers.c string_io_helpers.c xml_helpers.c tweet.c
cltwitter_update_cache_SOURCES = definitions.h application_helpers.c oauth_helpers.c string_io_helpers.c network_helpers.c xml_helpers.c cltwitter-update-cache.c

bashcompdir = $(sysconfdir)/bash_completion.d
bashcomp_DATA = completion/bash/tweet

EXTRA_DIST = $(bashcomp_DATA)