#ifndef NETWORK_HELPERS_H
#define NETWORK_HELPERS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include "definitions.h"
#include "string_io_helpers.h"

typedef struct {
  char* memory;
  size_t size;
} memory;

size_t ignore_data(void*, size_t, size_t, void*);
size_t write_to_memory(void*, size_t, size_t, void*);
char *shorten_url(char*);
char *response_message(unsigned long);

#endif /* NETWORK_HELPERS_H */
