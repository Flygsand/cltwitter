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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <curl/curl.h>

#define VERSION "20090401"
#define CONFIG_FILENAME ".cltwitter"
#define MAX_MESSAGE_LENGTH 140
#define MAX_USERNAME_PWD_LENGTH 100
#define TWITTER_UPDATE_URL "http://twitter.com/statuses/update.xml"

#define S(X) STRINGIFY(X)
#define STRINGIFY(X) #X

#define COMPLAIN_AND_EXIT(FORMAT, ...) { fprintf(stderr, FORMAT, ##__VA_ARGS__); exit(-1); }

#define TRUE 1
#define FALSE 0
typedef unsigned char bool;

typedef struct { 
  char username[MAX_USERNAME_PWD_LENGTH];
  char password[MAX_USERNAME_PWD_LENGTH];
} config;

config *parse_config(void);
char *url_encode(char*);

int main(int argc, char *argv[]) {
  
  /* require one argument */
  if (argc != 2)
    COMPLAIN_AND_EXIT("cltwitter, version " VERSION "\nUsage: tweet [message]\nNOTE: Make sure that [message] is surrounded by quotes!\n");
  
  /* check message length */
  int length = strlen(argv[1]);
  if (length == 0 || length > MAX_MESSAGE_LENGTH)
    COMPLAIN_AND_EXIT("Error: Message must be between 1 and " S(MAX_MESSAGE_LENGTH) " characters long.\n");
  
  /* format POST data */
  int size = length + 8; // "status=" + '\0'
  char data[size];
  char *url_encoded_status = url_encode(argv[1]);
  snprintf(data, size, "%s%s", "status=", url_encoded_status);
  free(url_encoded_status);
  
  /* load configuration */
  config *cfg = parse_config();
  
  if(!cfg)
    COMPLAIN_AND_EXIT("Error: Could not load configuration. Make sure that the " \
      "configuration file exists, is readable and contains the necessary " \
      "information (see the README for more information).\n");
  
  size = 2*MAX_USERNAME_PWD_LENGTH + 2; // ':' + '\0'
  char userpwd[size];
  snprintf(userpwd, size, "%s:%s", cfg->username, cfg->password);
  free(cfg);
  
  /* send update */
  CURL *curl = curl_easy_init();
  CURLcode res;
  
  if (curl) {
    
    FILE *dev_null = fopen("/dev/null", "w");
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, dev_null);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cltwitter (" VERSION ")");
    curl_easy_setopt(curl, CURLOPT_URL, TWITTER_UPDATE_URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    
    res = curl_easy_perform(curl);
    
    curl_easy_cleanup(curl);
    fclose(dev_null);
    
    if (res != CURLE_OK)
      COMPLAIN_AND_EXIT("Error: Twitter connection gave the following error \"%s\n\"", 
        curl_easy_strerror(res));
  }
  
  return 0;
}

config *parse_config() {
  config *cfg;
  FILE *fp; 
  char *cfg_dir = getenv("HOME");
  size_t size = strlen(cfg_dir) + strlen(CONFIG_FILENAME) + 2; // '/' + '\0'
  char *cfg_path = malloc(size);
  
  if (cfg_dir) {
    snprintf(cfg_path, size, "%s/%s", cfg_dir, CONFIG_FILENAME);
  } else {
    free(cfg_path);
    return NULL;
  }
  
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
    
    if (!(has_username && has_password))
      COMPLAIN_AND_EXIT("Error: Couldn't parse the configuration file. Please refer to the README file.\n");
    
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

