#ifndef STRING_IO_HELPERS_H
#define STRING_IO_HELPERS_H

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "definitions.h"

char *url_encode(char*);
char *get_line(FILE*);
char *trim(char*);
#endif /* STRING_IO_HELPERS_H */
