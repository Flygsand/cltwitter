#include "string_io_helpers.h"

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

char *get_line(FILE *stream) {
  size_t capacity = MAX_MESSAGE_LENGTH, remaining = capacity;
  char *line = malloc(capacity), *ptr = line, *expanded;
  char c;
  
  if (line == NULL)
    return NULL;
    
  c = fgetc(stream);
  while (c != EOF && c != '\n') {
    
    if (--remaining == 0) {
      remaining = capacity;
      expanded = realloc(line, capacity *= 2);
      
      if (expanded == NULL) {
        free(line);
        return NULL;
      }
      
      ptr = expanded + (ptr - line);
      line = expanded;
      
    }
    
    *(ptr++) = c;
    c = fgetc(stream);
  }
  
  return line;
}

char *trim(char *str) {
  char *ptr, *eos = NULL;
  char c;
  
  while (*str && isspace(*str))
    ++str;

  ptr = str;
  
  while (*ptr) {
    c = *(ptr++);
    if (!isspace(c)) eos = ptr;
  }
  
  if (eos != NULL) *eos = '\0';
  
  return str;
}
