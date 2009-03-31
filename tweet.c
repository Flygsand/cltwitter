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
char *url_encode(char*);

int main(int argc, char *argv[]) {
  
  /* require at least one argument */
  if (argc < 2) {
    printf("Usage: twitter [message]\n");
    return -1;
  } else if (argc > 2) {
    printf("Error: Too many arguments. Please enclose messages containing spaces in quotes.");
    return -1;
  }
  
  /* check message length */
  int length = strlen(argv[1]);
  if (length > MAX_MESSAGE_LENGTH) {
    printf("Error: Message can be at most %d characters long according to Twitter.\n", MAX_MESSAGE_LENGTH);
    return -1;
  }
  
  /* format POST data */
  int size = length + 7; // make space for "status=" 
  char data[size];
  char* url_encoded_status = url_encode(argv[1]);
  snprintf(data, size, "%s%s", "status=", url_encoded_status);
  free(url_encoded_status);
  
  /* load configuration */
  config *cfg = parse_config();
  
  if(!cfg) {
    printf("error: could not load configuration. make sure that the file ... exists, is readable and contains the necessary information (see the README for more information).\n");
    return -1;
  }
  
  size = strlen(cfg->username) + strlen(cfg->password) + 2; // string lengths + ':' + '\0'
  char userpwd[size];
  snprintf(userpwd, size, "%s:%s", cfg->username, cfg->password);
  free(cfg->username);
  free(cfg->password);
  free(cfg);
  
  /* send update */
  CURL *curl;
  CURLcode res;
  
  curl = curl_easy_init();
  
  if (curl) {
    
    FILE *dev_null = fopen("/dev/null", "w");
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, dev_null);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cltwitter (20090331)");
    curl_easy_setopt(curl, CURLOPT_URL, "http://twitter.com/statuses/update.xml");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK)
      printf("error: %s\n", curl_easy_strerror(res));
    
    curl_easy_cleanup(curl);
    fclose(dev_null);
  }
  
  return 0;
}

config *parse_config() {
  config *cfg;
  FILE *fp;
  char *cfg_path = getenv("HOME");
  static char cfg_file[] = ".cltwitter";
 
  if (cfg_path) {
    snprintf(cfg_path, strlen(cfg_path) + strlen(cfg_file) + 1, "%s/%s", cfg_path, cfg_file);
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
    
    fclose(fp);
    return cfg;
  } else {
    return NULL;
  }
}

char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

