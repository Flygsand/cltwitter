#ifndef XML_HELPERS_H
#define XML_HELPERS_H

#include <libxml/parser.h>
#include <libxml/xpath.h>

xmlXPathObjectPtr eval_xpath(xmlDocPtr, xmlChar*);

#endif /* XML_HELPERS_H */
