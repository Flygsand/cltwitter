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

#include "definitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <pcre.h>
#include "application_helpers.h"
#include "network_helpers.h"
#include "string_io_helpers.h"
#include "xml_helpers.h"

int main(int argc, char *argv[]) {
  /* definitions */
  cltwitter_mode mode;
  size_t length = 0, url_length = 0, shortened_url_length = 0;
  char *input, *trimmed_input, *url, *shortened_url, *url_encoded_status, *err_msg, 
       data[DATA_LENGTH], *signed_update_url;
  config *cfg = NULL;
  token *tok = NULL;
  CURL *curl;
  CURLcode res;
  struct curl_httppost *post = NULL, *last = NULL;
  unsigned long response_code;
  const char *regex_errmsg;
  int regex_result, regex_match_offset, regex_err_offset, match[2];
  pcre *regexp;
  xmlDocPtr doc;
  xmlXPathObjectPtr xpath_result;
  memory twitpic_xml = {NULL, 0};
  int upload_only_flag_position = find_flag(UPLOAD_ONLY_FLAG, argc, argv);
  
  /* parse command line arguments */
  if (
    (upload_only_flag_position > 0 && upload_only_flag_position < 3)
    || (argc == 4 && upload_only_flag_position == -1)
    || argc > 4) {
    COMPLAIN_AND_EXIT("cltwitter, version " VERSION "\nUsage: tweet [message [image_path [--upload-only]]]" \
                      "\n\nExplanation:\n" \
                      "message - the message to be posted to Twitter. Messages containing spaces must be quoted.\n" \
                      "image_path - specifies the path to an image on the local filesystem that will be uploaded to Twitpic.\n" \
                      "--upload-only - upload image to Twitpic, but do not post to Twitter. Only valid if image_path is given.\n" \
                      "\nIf no arguments are given, the message argument will be read from standard input.\n");
  } else if (argc < 2) {
    mode = CLTWITTER_STDIN;
    input = get_line(stdin);
    tok = get_access_token();
  } else if (argc == 2) {
    mode = CLTWITTER_ARG;
    input = argv[1];
    tok = get_access_token();
  } else if (argc == 3) {
    mode = CLTWITTER_TWITPIC;
    input = argv[1];
    cfg = parse_config();
  } else if (argc == 4 && upload_only_flag_position == 3) {
    mode = CLTWITTER_TWITPIC_UPLOAD_ONLY;
    input = argv[1];
    cfg = parse_config();
  }
  
  /* load configuration */
  if((mode == CLTWITTER_STDIN || mode == CLTWITTER_ARG) && !tok) {
    if (mode == CLTWITTER_STDIN) free(input);
    COMPLAIN_AND_EXIT("Error: Failed to get OAuth access token. Make sure that you provided the correct credentials.\n");
  }
  
   /* load configuration */
  if((mode == CLTWITTER_TWITPIC || mode == CLTWITTER_TWITPIC_UPLOAD_ONLY) && !cfg) {
    COMPLAIN_AND_EXIT("Error: Could not load or parse configuration. Make sure that the " \
                      "configuration file exists, is readable and contains the necessary " \
                      "information (see the README for more information).\n");
  }
  
  /* remove leading/trailing whitespace from input */
  trimmed_input = trim(input); 
  length = strlen(trimmed_input);
  
  /* shorten URLs */
  regex_match_offset = 0;
  regexp = pcre_compile(URL_REGEX, PCRE_CASELESS, &regex_errmsg, &regex_err_offset, NULL);
  if (regexp) {
    regex_result = pcre_exec(regexp, NULL, trimmed_input, length, regex_match_offset, PCRE_NOTEMPTY, match, 2);
    while (regex_result >= 0) {
      url_length = match[1] - match[0];
      url = calloc(url_length + 1, sizeof(char));
      strncpy(url, trimmed_input + match[0], url_length);
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

    pcre_free(regexp);
  } else {
    fprintf(stderr, "Notice: compilation of regular expression for URLs failed. Not shortening URLs.\n"); 
  }
  
  free((void*)regex_errmsg);
  
  /* check message length */
  if (length == 0 || length > MAX_MESSAGE_LENGTH) {
    if (tok) free(tok);
    if (cfg) free(cfg);
    COMPLAIN_AND_EXIT("Error: Message must be between 1 and " S(MAX_MESSAGE_LENGTH) " characters long.\n");
  }
  
  /* send update */
  curl = curl_easy_init();
  
  if (!curl) {
    if (tok) free(tok);
    if (cfg) free(cfg);
    COMPLAIN_AND_EXIT("Error: Couldn't init connection mechanism. Tweet not sent.\n");
  }
  
  curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT_HEADER); 
    
  if (mode == CLTWITTER_TWITPIC || mode == CLTWITTER_TWITPIC_UPLOAD_ONLY) {
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "media", CURLFORM_FILE, argv[2], CURLFORM_END);
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "message", CURLFORM_PTRCONTENTS, trimmed_input, CURLFORM_END);
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "username", CURLFORM_PTRCONTENTS, cfg->username, CURLFORM_END);
    curl_formadd(&post, &last, CURLFORM_COPYNAME, "password", CURLFORM_PTRCONTENTS, cfg->password, CURLFORM_END);

    if (mode == CLTWITTER_TWITPIC_UPLOAD_ONLY)
      curl_easy_setopt(curl, CURLOPT_URL, TWITPIC_UPLOAD_URL);
    else
      curl_easy_setopt(curl, CURLOPT_URL, TWITPIC_POST_URL);
    
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &twitpic_xml);
  
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    free(cfg);
    
    if (res != CURLE_OK) {
      if (twitpic_xml.memory) free(twitpic_xml.memory);
      COMPLAIN_AND_EXIT("(Twitpic) Error: %s\n", curl_easy_strerror(res));
    }
    
    if (!(response_code == OK || response_code == NOT_MODIFIED)) {
      if (twitpic_xml.memory) free(twitpic_xml.memory);
      COMPLAIN_AND_EXIT("(Twitpic) Error: %s (#%lu)\n", response_message(response_code), response_code);
    }
    
    /* parse response XML */
    doc = xmlParseMemory(twitpic_xml.memory, twitpic_xml.size); 
    free(twitpic_xml.memory);
    
    if (!doc)
      COMPLAIN_AND_EXIT("(Twitpic) Warning: Unable to parse Twitpic's XML response. There might have been an error.\n");
    
    xpath_result = eval_xpath(doc, (xmlChar*) "//err");
    
    if (xpath_result) { // error response from Twitpic
      err_msg = (char*)xmlGetProp(xpath_result->nodesetval->nodeTab[0], (xmlChar*) "msg");
      xmlXPathFreeObject(xpath_result);
      xmlFreeDoc(doc);
      xmlCleanupParser();
      fprintf(stderr, "(Twitpic) Error: %s\n", err_msg);
      xmlFree(err_msg);  
      exit(-1);
    }
    
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
  } else {
    url_encoded_status = url_encode(trimmed_input);
    SNPRINTF(data, DATA_LENGTH, "%s%s", "status=", url_encoded_status);
    free(url_encoded_status);
    
    if (mode == CLTWITTER_STDIN) free(input);
    
    signed_update_url = oauth_sign_url(TWITTER_UPDATE_URL, NULL, OA_HMAC, OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET, tok->key, tok->secret);
    free(tok);
    printf("URL: %s\n", signed_update_url);
    
    if (!signed_update_url) { curl_easy_cleanup(curl); COMPLAIN_AND_EXIT("Error: Signing of OAuth request URL failed. Tweet not sent.\n"); } 
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ignore_data);
    curl_easy_setopt(curl, CURLOPT_URL, signed_update_url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT_HEADER);
    
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    free(signed_update_url);
    if (res != CURLE_OK)
      COMPLAIN_AND_EXIT("(Twitter) Error: %s\n", curl_easy_strerror(res));
    if (!(response_code == OK || response_code == NOT_MODIFIED))
      COMPLAIN_AND_EXIT("(Twitter) Error: %s (#%lu)\n", response_message(response_code), response_code);
  }
  
  return 0;
}
