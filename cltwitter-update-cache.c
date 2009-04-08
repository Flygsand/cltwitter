#include "definitions.h"
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string.h>
#include <errno.h>
#include "application_helpers.h"
#include "network_helpers.h"
#include "xml_helpers.h"

int main(int argc, char *argv[]) {
  char userpwd[USERPWD_LENGTH];
  char *cache_data, *new_cache_data, *cache_ptr, *cache_file_path, *screen_name;
  size_t cache_size, old_cache_size;
  long cache_space;
  config *cfg;
  CURL *curl;
  CURLcode res;
  unsigned long response_code;
  memory friends_xml = {NULL, 0};
  xmlDocPtr doc;
  xmlXPathObjectPtr xpath_result;
  xmlNodeSetPtr nodeset;
  FILE* cache_file;
  unsigned int i, length;
  
  /* load configuration */
  cfg = parse_config();
  
  if(!cfg)
    COMPLAIN_AND_EXIT("Error: Could not load or parse configuration. Make sure that the " \
                      "configuration file exists, is readable and contains the necessary " \
                      "information (see the README for more information).\n");
  
  SNPRINTF(userpwd, USERPWD_LENGTH, "%s:%s", cfg->username, cfg->password);
  free(cfg);

  /* retrieve friends XML */
  curl = curl_easy_init();
  
  if (curl) {  
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT_HEADER);
    curl_easy_setopt(curl, CURLOPT_URL, TWITTER_FRIENDS_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &friends_xml);
    
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    
    if (!friends_xml.memory) 
      COMPLAIN_AND_EXIT("Error: Received no data from Twitter.\n");
      
    if (res != CURLE_OK) {
      free(friends_xml.memory);
      COMPLAIN_AND_EXIT("(Twitter) Error: %s\n", curl_easy_strerror(res));
    }
    
    if (!(response_code == OK || response_code == NOT_MODIFIED)) {
      free(friends_xml.memory);
      COMPLAIN_AND_EXIT("(Twitter) Error: %s (#%lu)\n", response_message(response_code), response_code);
    }
      
    /* parse friends XML */
    doc = xmlParseMemory(friends_xml.memory, friends_xml.size); 
    free(friends_xml.memory);
    
    if (!doc)
      COMPLAIN_AND_EXIT("Error: Unable to parse Twitter's XML response.\n");
      
    xpath_result = eval_xpath(doc, (xmlChar*) "//screen_name");
    
    if (!xpath_result) {  
      xmlFreeDoc(doc);
      xmlCleanupParser();
      return 0; // no data to process - should perhaps empty cache
    } 
    
    nodeset = xpath_result->nodesetval;
    cache_size = nodeset->nodeNr*15; // assuming each screen name being avg. 15 chars long
    cache_space = cache_size;
    cache_data = malloc(cache_size); 
    cache_ptr = cache_data;
    
    /* prepare data for writing */
    for (i = 0; i < nodeset->nodeNr; i++) {
      screen_name = (char*)xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
      length = strlen(screen_name) + 2;
      
      cache_space -= length;
      if (cache_space <= 0) {
        old_cache_size = cache_size;
        cache_size = (cache_size*2 > (cache_size + length)) ? cache_size*2 : (cache_size + length);
        cache_space = cache_space + (cache_size - old_cache_size);
        
        new_cache_data = realloc(cache_data, cache_size);
        if (!new_cache_data) {
          free(cache_data);
          xmlXPathFreeObject(xpath_result);
          xmlFreeDoc(doc);
          xmlCleanupParser();          
          COMPLAIN_AND_EXIT("Error: Could not allocate memory.\n");
        }
        cache_ptr = new_cache_data + (cache_ptr - cache_data);
        cache_data = new_cache_data;
      }
      
      if (i == 0) {
        strncpy(cache_ptr, screen_name, length - 1);
        cache_ptr += (length-2)*sizeof(char);
      } else {
        SNPRINTF(cache_ptr, length, " %s", screen_name);
        cache_ptr += (length-1)*sizeof(char);
      }
      
      free(screen_name);    
    }
    
    xmlXPathFreeObject(xpath_result);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    *cache_ptr = '\0';
    
    /* write data to cache file */
    cache_file_path = get_absolute_path(CACHE_FILENAME);
    cache_file = fopen(cache_file_path, "w+");
    
    if (!cache_file)
      COMPLAIN_AND_EXIT("Error: Could not open cache file (%s)\n", strerror(errno));
    
    fputs(cache_data, cache_file);
    fclose(cache_file);
    free(cache_file_path);
 
  }
  
  return 0;
}
