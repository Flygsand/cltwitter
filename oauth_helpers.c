#include "oauth_helpers.h"

int oauth_param_exists(char **argv, int argc, char *param) {
	int i;
	size_t l= strlen(param);
	for (i=0;i<argc;i++)
		if (strlen(argv[i])>l && !strncmp(argv[i],param,l) && argv[i][l] == '=') return 1;
	return 0;
}

char *my_oauth_sign_url (const char *url, int argc, char **postargs, 
  OAuthMethod method, 
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key 
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {

  char **argv = (char**)malloc((argc+=1)*sizeof(char*));
  char *tmp;
  char oarg[1024];
  char *query;
  char *okey, *odat, *sign;
  char *result;
  int i;
   
  
  argv[0] = calloc(strlen(url)+1, sizeof(char));
  strcpy(argv[0], url);
  for (i = 1; i < argc; i++) argv[i] = strdup(postargs[i-1]);
  
#define ADD_TO_ARGV \
  argv=(char**) realloc(argv,sizeof(char*)*(argc+1)); \
  argv[argc++]=strdup(oarg); 
  // add oAuth specific arguments
	if (!oauth_param_exists(argv,argc,"oauth_nonce")) {
		snprintf(oarg, 1024, "oauth_nonce=%s", (tmp=oauth_gen_nonce()));
		ADD_TO_ARGV;
		free(tmp);
	}

	if (!oauth_param_exists(argv,argc,"oauth_timestamp")) {
		snprintf(oarg, 1024, "oauth_timestamp=%li", time(NULL));
		ADD_TO_ARGV;
	}

	if (t_key) {
    snprintf(oarg, 1024, "oauth_token=%s", t_key);
    ADD_TO_ARGV;
  }

  snprintf(oarg, 1024, "oauth_consumer_key=%s", c_key);
  ADD_TO_ARGV;

  snprintf(oarg, 1024, "oauth_signature_method=%s",
      method==0?"HMAC-SHA1":method==1?"RSA-SHA1":"PLAINTEXT");
  ADD_TO_ARGV;

	if (!oauth_param_exists(argv,argc,"oauth_version")) {
		snprintf(oarg, 1024, "oauth_version=1.0");
		ADD_TO_ARGV;
	}

  // sort parameters
  qsort(&argv[1], argc-1, sizeof(char *), oauth_cmpstringp);

  // serialize URL
  query= oauth_serialize_url_parameters(argc, argv);

  // generate signature
  okey = oauth_catenc(2, c_secret, t_secret);
  odat = oauth_catenc(3, postargs?"POST":"GET", argv[0], query);

  switch(method) {
    case OA_RSA:
      sign = oauth_sign_rsa_sha1(odat,okey); // XXX okey needs to be RSA key!
    	break;
    case OA_PLAINTEXT:
      sign = oauth_sign_plaintext(odat,okey);
    	break;
    default:
      sign = oauth_sign_hmac_sha1(odat,okey);
  }

  free(odat); 
  free(okey);

  // append signature to query args.
  snprintf(oarg, 1024, "oauth_signature=%s",sign);
  ADD_TO_ARGV;
  free(sign);

  // build URL params
  result = oauth_serialize_url(argc, (postargs?1:0), argv);

  if(postargs) { 
    *postargs = result;
    result = strdup(argv[0]);
    free(argv[0]);
  }
  if(argv) free(argv);
  if(query) free(query);

  return result;
}

int parse_reply(const char *reply, char **token, char **secret) {
  int rc;
  int ok = 1;
  char **rv = NULL;
  rc = oauth_split_url_parameters(reply, &rv);

  if( rc >= 2
      && !strncmp(rv[0],"oauth_token=", 11)
      && !strncmp(rv[1],"oauth_token_secret=",18) ) {
    ok = 0;
    if (token)  *token = strdup(&(rv[0][12]));
    if (secret) *secret = strdup(&(rv[1][19]));
  }
  if(rv) free(rv);
  return ok;
}

token *get_access_token() {
  char *token_path = get_absolute_path(TOKEN_FILENAME), *req_url = NULL, 
    *reply = NULL, line[1024], c, *authorize_url, *browser, *file_data, 
    *postargs = NULL, *t_key = NULL, *t_secret = NULL;
  size_t size;
  int i;
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
    req_url = oauth_sign_url2(OAUTH_REQUEST_TOKEN_URL, &postargs, OA_HMAC, NULL, OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET, NULL, NULL);
    
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
           "this once. cltwitter will now try to open your browser and point you to twitter.com.\n");
           
    if (!browser) {
      printf("\nNotice: Could not detect your browser. Please go to this URL to complete the process: %s\n", authorize_url);
    } else {
      system(browser);
      free(browser);
    }
    
    free(authorize_url);
    
    printf("\nEnter the 7 digit PIN code provided by Twitter: ");
    

    while (TRUE) {
      i = 0;
      while((c = getchar()) != '\n' && c != EOF && i < 1023) {
        line[i] = c;
        ++i;
      }
      
      if (i == 7) {
        line[i] = '\0';
        break;
      } else {
        printf("Given PIN code is not 7 digits long, please try again: ");
      }
    }
    
    size = sizeof(OAUTH_ACCESS_TOKEN_URL) + 24;
    req_url = (char *) malloc(size);
    SNPRINTF(req_url, size, "%s?oauth_verifier=%s", OAUTH_ACCESS_TOKEN_URL, line);

    req_url = oauth_sign_url2(req_url, &postargs, OA_HMAC, NULL, OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET, t_key, t_secret);  // segfault?
    
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
