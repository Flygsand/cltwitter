#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifdef _WIN32
  #define CURL_STATICLIB
  #define SNPRINTF _snprintf
  #define HOME "USERPROFILE"
  #define DS "\\"
#else
  #define SNPRINTF snprintf
  #define HOME "HOME"
  #define DS "/"
#endif

#define VERSION "20090405"
#define CONFIG_FILENAME ".cltwitter"
#define MAX_MESSAGE_LENGTH 140
#define MAX_USERNAME_PWD_LENGTH 100
#define TWITTER_UPDATE_URL "http://twitter.com/statuses/update.xml"
#define IS_GD_API "http://is.gd/api.php?longurl="
#define URL_REGEX "\\b(\?:(\?:https\?|ftp|file)://|www\\.|ftp\\.)(\?:\\([-A-Z0-9+&@#/%=~_|$\?!:,.]*\\)|[-A-Z0-9+&@#/%=~_|$\?!:,.])*(\?:\\([-A-Z0-9+&@#/%=~_|$\?!:,.]*\\)|[A-Z0-9+&@#/%=~_|$])"

#define DATA_LENGTH 3*MAX_MESSAGE_LENGTH + 8
#define USERPWD_LENGTH 2*MAX_USERNAME_PWD_LENGTH + 2

#define S(X) STRINGIFY(X)
#define STRINGIFY(X) #X

#define COMPLAIN_AND_EXIT(FORMAT, ...) { fprintf(stderr, FORMAT, ##__VA_ARGS__); exit(-1); }

#define TRUE 1
#define FALSE 0
typedef unsigned char bool;

enum http_response_code {
  OK = 200,
  NOT_MODIFIED = 304,
  BAD_REQUEST = 400,
  NOT_AUTHORIZED = 401,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  INTERNAL_SERVER_ERROR = 500,
  BAD_GATEWAY = 502,
  SERVICE_UNAVAILABLE = 503
};

#endif /* DEFINITIONS_H */
