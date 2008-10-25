/*

cltwitter - a command-line utility for Twitter

Copyright 2008 Martin Häger <martin.haeger@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_MESSAGE_LENGTH 140

typedef struct { 
  char username[100];
  char password[100];
} config;

config *parse_config(void);

int main(int argc, char *argv[]) {
  
  /* require at least one argument */
  if (argc < 2) {
    printf("usage: twitter [message]\n");
    return -1;
  } else if (argc > 2) {
    printf("warning: ignored additional arguments. please enclose messages containing spaces in quotes.");
  }
  
  /* message length limit */
  int length = strlen(argv[1]);
  
  if (length > MAX_MESSAGE_LENGTH) {
    printf("error: message can be at most %d characters long.\n", MAX_MESSAGE_LENGTH);
    return -1;
  }
  
  /* format POST data */
  char data[length + 7];  
  strcpy(data, "status=");
  strcat(data, argv[1]);
  
  /* load configuration */
  char userpwd[200];
  config *cfg = parse_config();
  
  if(!cfg) {
    printf("error: could not load configuration. make sure that the file ... exists, is readable and contains the necessary information (see the README for more information).\n");
    return -1;
  }

  strcpy(userpwd, cfg->username);
  strcat(userpwd, ":");
  strcat(userpwd, cfg->password);
  
  /* send update */
  CURL *curl;
  CURLcode res;
  
  curl = curl_easy_init();
  
  if (curl) {
    
    FILE *dev_null = fopen("/dev/null", "w");
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, dev_null);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cltwitter (0.1+20081025)");
    curl_easy_setopt(curl, CURLOPT_URL, "http://twitter.com/statuses/update.xml");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK)
      printf("error: %s\n", curl_easy_strerror(res));
    
    curl_easy_cleanup(curl);
  }
  
  return 0;
}

config *parse_config() {
  config *cfg;
  FILE *fp;
  char *cfg_path;
  
  if (getenv("HOME")) {
    cfg_path = getenv("HOME");
    strcat(cfg_path, "/.cltwitter");
  } else {
    return NULL;
  }
  
  if ( (fp = fopen(cfg_path, "r")) &&
       (cfg = (config*)malloc(sizeof(config))) ) {
    
    char line[128];
    
    while (fgets(line, sizeof line, fp) != NULL) {
      sscanf(line, "username=%s", cfg->username); 
      sscanf(line, "password=%s", cfg->password);
    }
    
    return cfg;
  } else {
    return NULL;
  }
}
