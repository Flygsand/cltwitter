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

#ifndef XML_HELPERS_H
#define XML_HELPERS_H

#include <libxml/parser.h>
#include <libxml/xpath.h>

xmlXPathObjectPtr eval_xpath(xmlDocPtr, xmlChar*);

#endif /* XML_HELPERS_H */
