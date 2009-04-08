#include "application_helpers.h"

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
