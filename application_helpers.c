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

#include "application_helpers.h"

int find_flag(char *flag, int argc, char *argv[]) {
  int pos;
  
  for (pos = 1; pos < argc; pos++) {
    if (strcmp(flag, argv[pos]) == 0) 
      return pos;
  }
  
  return -1;
}

char *get_absolute_path(const char* filename) {
  char *cfg_path, *cfg_dir = getenv(HOME);
  size_t size;
  
  if (!cfg_dir)
    return NULL;
  
  size = strlen(cfg_dir) + strlen(filename) + 2; // '/' + '\0'
  cfg_path = malloc(size);
  SNPRINTF(cfg_path, size, "%s%s%s", cfg_dir, DS, filename);
  return cfg_path;
}

char *get_browser_cmd(char *url) {
  #ifdef _WIN32
    return NULL;
  #else
  const int browserc = 3;
  const size_t cmd_len = 64;
  char *browsers[] = {"firefox", "opera", "safari"};
  char *res;
  char *cmd = calloc(cmd_len, sizeof(char));
  int i;
  
  for (i = 0; i < browserc; i++) {
    SNPRINTF(cmd, cmd_len, "which %s > /dev/null", browsers[i]);
  
    if (system(cmd) == 0) {
      res = calloc(1024, sizeof(char));
      SNPRINTF(res, 1024, "%s %s", browsers[i], url);
      free(cmd); return res;
    }
  }
  
  free(cmd);
  return NULL; 
  #endif
}

config *parse_config() {
  char *cfg_path = get_absolute_path(CONFIG_FILENAME);
  FILE *fp;
  config *cfg;
  
  fp = fopen(cfg_path, "r");
  cfg = malloc(sizeof(config));
  
  if (fp && cfg) {
    char line[1024];
    bool has_username, has_password;
    has_username = has_password = FALSE;
    while (fgets(line, sizeof line, fp) != NULL) {
      if (sscanf(line, "username=%" S(MAX_USERNAME_PWD_LENGTH) "s", cfg->username))
        has_username = TRUE;
      if(sscanf(line, "password=%" S(MAX_USERNAME_PWD_LENGTH) "s", cfg->password))
        has_password = TRUE;
    }
    
    free(cfg_path);
    fclose(fp);
    
    if (!(has_username && has_password)) {
      free(cfg);
      return NULL;
    }
    
    return cfg;
  } else {
    if (fp)
      fclose(fp);
    if (cfg)
      free(cfg);
      
    free(cfg_path);
    return NULL;
  }
}
