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

#include "definitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <curl/curl.h>
#include <pcre.h>
#include "network_helpers.h"
#include "string_io_helpers.h"

typedef struct { 
  char username[MAX_USERNAME_PWD_LENGTH];
  char password[MAX_USERNAME_PWD_LENGTH];
} config;

config *parse_config(void);

int main(int argc, char *argv[]) {
  size_t length = 0, url_length = 0, shortened_url_length = 0;
  char *input, *trimmed_input, *url, *shortened_url, *url_encoded_status;
  char data[DATA_LENGTH];
  char userpwd[USERPWD_LENGTH];
  config *cfg;
  CURL *curl;
  CURLcode res;
  unsigned long response_code;
  const char* regex_errmsg;
  int regex_result, regex_match_offset, regex_err_offset;
  int match[2];
  pcre *regexp = pcre_compile(URL_REGEX, PCRE_CASELESS, &regex_errmsg, &regex_err_offset, NULL);
  
  /* read input, either from argv or stdin */ 
  if (argc < 2) {
    input = get_line(stdin);
  } else if (argc == 2) {
    input = argv[1];
  } else {
    COMPLAIN_AND_EXIT("cltwitter, version " VERSION "\nUsage: tweet [message]\nNOTE: Make sure " \
                      "that [message] is surrounded by quotes. If [message] is not given, data " \
                      "will be read from standard input.\n");
  }
  
  /* load configuration */
  cfg = parse_config();
  
  if(!cfg) {
    if (argc < 2) free(input);
    COMPLAIN_AND_EXIT("Error: Could not load or parse configuration. Make sure that the " \
                      "configuration file exists, is readable and contains the necessary " \
                      "information (see the README for more information).\n");
  }
  
  SNPRINTF(userpwd, USERPWD_LENGTH, "%s:%s", cfg->username, cfg->password);
  free(cfg);
  
  /* remove leading/trailing whitespace from input */
  trimmed_input = trim(input); 
  length = strlen(trimmed_input);
  
  /* shorten URLs */
  regex_match_offset = 0;
  regex_result = pcre_exec(regexp, NULL, trimmed_input, length, regex_match_offset, PCRE_NOTEMPTY, match, 2);
  while (regexp && regex_result >= 0) {
    url_length = match[1] - match[0];
    url = calloc(url_length + 1, sizeof(char));
    strncpy(url, trimmed_input + match[0], url_length*sizeof(char));
    regex_match_offset = match[1];
    
    shortened_url = shorten_url(url);
    shortened_url_length = strlen(shortened_url);
    
    if (shortened_url_length < url_length) { // only use shortened URL if it's actually shorter
      strcpy(trimmed_input + match[0], shortened_url); // copy shortened url into place
      strcpy(trimmed_input + match[0] + shortened_url_length, trimmed_input + match[1]); // copy chars after URL
      length = length - url_length + shortened_url_length;
      regex_match_offset = regex_match_offset - url_length + shortened_url_length;
    }
    
    free(url);
    free(shortened_url);
    
    regex_result = pcre_exec(regexp, NULL, trimmed_input, length, regex_match_offset, PCRE_NOTEMPTY, match, 2);  
  }
  
  if (regexp)
    pcre_free(regexp);
  
  /* check message length */
  if (length == 0 || length > MAX_MESSAGE_LENGTH)
    COMPLAIN_AND_EXIT("Error: Message must be between 1 and " S(MAX_MESSAGE_LENGTH) " characters long.\n");
  
  /* format POST data */
  url_encoded_status = url_encode(trimmed_input);
  SNPRINTF(data, DATA_LENGTH, "%s%s", "status=", url_encoded_status);
  free(url_encoded_status);
  if (argc < 2) free(input);

  /* send update */
  curl = curl_easy_init();
  
  if (curl) {  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ignore_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cltwitter (" VERSION ")");
    curl_easy_setopt(curl, CURLOPT_URL, TWITTER_UPDATE_URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
      COMPLAIN_AND_EXIT("(Twitter) Error: %s\n", curl_easy_strerror(res));
    if (!(response_code == OK || response_code == NOT_MODIFIED))
      COMPLAIN_AND_EXIT("(Twitter) Error: %s (#%lu)\n", response_message(response_code), response_code);
    
  }
  
  return 0;
}

config *parse_config() {
  char *cfg_dir = getenv(HOME);
  size_t size = strlen(cfg_dir) + strlen(CONFIG_FILENAME) + 2; // '/' + '\0'
  char *cfg_path = malloc(size);
  FILE *fp; 
  config *cfg;
  
  if (cfg_dir) {
    SNPRINTF(cfg_path, size, "%s%s%s", cfg_dir, DS, CONFIG_FILENAME);
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
