#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.55])

AC_INIT([cltwitter], [git], [martin.haeger@gmail.com])
AM_INIT_AUTOMAKE([cltwitter], [git])
AC_CONFIG_SRCDIR([string_io_helpers.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(xml2, xmlFree, [CFLAGS+=" `xml2-config --cflags`" LIBS+=" `xml2-config --libs`"], [AC_MSG_ERROR(libxml2 not found.)])
AC_CHECK_LIB(curl, curl_easy_init, [], [AC_MSG_ERROR(libpcre not found.)])
AC_CHECK_LIB(pcre, pcre_compile, [], [AC_MSG_ERROR(libpcre not found.)])
AC_CHECK_LIB(oauth, oauth_sign_rsa_sha1, [], [AC_MSG_ERROR(liboauth not found.)])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
