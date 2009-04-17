/*

This file is part of cltwitter - a command-line utility for Twitter

Copyright 2008, 2009 Martin Häger <martin.haeger@gmail.com>

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

#ifndef APPLICATION_HELPERS_H
#define APPLICATION_HELPERS_H
#include "definitions.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
  #include <sys/stat.h>
  #include <sys/types.h>
#endif
#include <string.h>
#include <oauth.h>
#include "oauth_helpers.h"

int find_flag(char*, int, char**);
char *get_absolute_path(const char*);
char *get_browser_cmd();
token *get_access_token(void);
config *parse_config(void);

#endif /* APPLICATION_HELPERS_H */
