#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
  #include <sys/stat.h>
  #include <sys/types.h>
#endif
#include <string.h>
#include <time.h>
#include <oauth.h>
#include "definitions.h"
#include "application_helpers.h"
#include "oauth_secret.h"

token *get_access_token(void);
char *my_oauth_sign_url (const char *url, int argc, char **postargs, 
  OAuthMethod method, const char *c_key, const char *c_secret, 
  const char *t_key, const char *t_secret);
