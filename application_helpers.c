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

int parse_reply(const char *reply, char **token, char **secret) {
  int rc;
  int ok=1;
  char **rv = NULL;
  rc = oauth_split_url_parameters(reply, &rv);
  qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
  if( rc==2 
      && !strncmp(rv[0],"oauth_token=",11)
      && !strncmp(rv[1],"oauth_token_secret=",18) ) {
    ok=0;
    if (token)  *token =strdup(&(rv[0][12]));
    if (secret) *secret=strdup(&(rv[1][19]));
  }
  if(rv) free(rv);
  return ok;
}

token *get_access_token() {
  char *token_path = get_absolute_path(TOKEN_FILENAME), *req_url = NULL, *reply = NULL, line[1024], c,
        *authorize_url, *browser, *file_data, *postargs = NULL, *t_key = NULL, *t_secret = NULL;
  bool has_access_token_key, has_access_token_secret;
  FILE *fp; 
  token *tok;
  #ifndef _WIN32
    mode_t old_umask;
  #endif
  
  if (!token_path)
    return NULL;
  
  fp = fopen(token_path, "r");
  
  if (fp) {
    tok = malloc(sizeof(token));
    tok->key = malloc(MAX_TOKEN_LENGTH);
    tok->secret = malloc(MAX_TOKEN_LENGTH);
    has_access_token_key = has_access_token_secret = FALSE;
    while (fgets(line, sizeof line, fp) != NULL) {
      if (sscanf(line, "access_token_key=%" S(MAX_TOKEN_LENGTH) "s", tok->key))
        has_access_token_key = TRUE;
      if(sscanf(line, "access_token_secret=%" S(MAX_TOKEN_LENGTH) "s", tok->secret))
        has_access_token_secret = TRUE;
    }
    
    free(token_path);
    fclose(fp);
    
    if (!(has_access_token_key && has_access_token_secret)) {
     free(tok); free(tok->key); free(tok->secret); return NULL;
    }
    
    return tok;
  } else {
    req_url = oauth_sign_url(OAUTH_REQUEST_TOKEN_URL, &postargs, OA_HMAC, OAUTH_CONSUMER_KEY, consumer_secret(), NULL, NULL);
    
    if (!req_url) {
      free(token_path); return NULL;
    }
    
    reply = oauth_http_post(req_url, postargs);
    if (postargs) free(postargs);
    free(req_url);
    if (parse_reply(reply, &t_key, &t_secret)) { free(token_path); return NULL; }
    free(reply);
    
    authorize_url = calloc(1024, sizeof(char));
    if (!authorize_url) { free(token_path); return NULL; }
    
    SNPRINTF(authorize_url, 1024, S(OAUTH_AUTHORIZE_URL) "?oauth_token=%s", t_key);
    browser = get_browser_cmd(authorize_url);
    printf("You need to allow cltwitter access to your Twitter profile. You will only need to do " \
           "this once. cltwitter will now try to open your browser and point you to Twitter's " \
           "authorization process. After you're done, press ENTER to continue.\n");
           
    if (!browser) {
      printf("\nNotice: Could not detect your browser. Please go to this URL to complete the process: %s\n", authorize_url);
    } else {
      system(browser);
      free(browser);
    }
    
    free(authorize_url);

    while((c = getchar()) != '\n' && c != EOF) ;
    
    req_url = oauth_sign_url(OAUTH_ACCESS_TOKEN_URL, &postargs, OA_HMAC, OAUTH_CONSUMER_KEY, consumer_secret(), t_key, t_secret);  // segfault?
    
    if (!req_url) { free(token_path); return NULL; }
    
    reply = oauth_http_post(req_url, postargs);
    free(req_url);
    if(t_key) free(t_key);
    if(t_secret) free(t_secret);
    if (postargs) free(postargs);
    if (!reply) { free(token_path); return NULL; }
    
    tok = malloc(sizeof(token));
    if (!tok) { free(token_path); free(reply); return NULL; }
    if (parse_reply(reply, &tok->key, &tok->secret)) { free(token_path); free(reply); free(tok); return NULL; }
    free(reply);
    
    #ifndef _WIN32
      old_umask = umask(077);
    #endif
    
    fp = fopen(token_path, "w");
    
    #ifndef _WIN32
      umask(old_umask);
    #endif
    
    file_data = calloc(1024, sizeof(char));
    free(token_path);
    
    if (fp && file_data) {
      SNPRINTF(file_data, 1024, "access_token_key=%s\naccess_token_secret=%s", tok->key, tok->secret);
      fputs(file_data, fp);
      free(file_data);
      fclose(fp);
    } else {
      if(fp) fclose(fp);
      if(file_data) free(file_data);
    }
    
    return tok;
  }
  
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
