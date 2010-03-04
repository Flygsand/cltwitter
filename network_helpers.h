/*

This file is part of cltwitter - a command-line utility for Twitter

Copyright 2008-2010 Martin Häger <martin.haeger@gmail.com>

cltwitter is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

cltwitter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with cltwitter.  If not, see <http://www.gnu.org/licenses/>.

*/

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
  size_t capacity;
} memory;

size_t ignore_data(void*, size_t, size_t, void*);
size_t write_to_memory(void*, size_t, size_t, void*);
char *shorten_url(char*);
char *response_message(unsigned long);

#endif /* NETWORK_HELPERS_H */
