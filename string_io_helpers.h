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
