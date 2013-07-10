/*

   Scanner for the configuration file, used to break the file into tokens.

   ==========================================================================

   Copyright (c) 2012 Daniel Baimiachkine

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

*/

#ifndef CONF_SCANNER_H
#define CONF_SCANNER_H

#include "strutil.h"
#include "gentype.h"

#define CONF_STRING_DELIM '"'
#define CONF_VALUE_KEY_SEPARATOR '='

#define CONF_COMMEND_START '#'
#define CONF_COMMENT_END '\n'
#define CONF_LINE_SEPARATOR '\n'

/* Scanner states */
typedef enum {
   STATE_START = 1,
   STATE_KEYWORD,
   STATE_STRING,
   STATE_COMMENT
} ConfTokenState;

typedef unsigned int ConfTokenPosition;

/* A config scanner client is a structure used to store intermediate 
   information to help the scanner tokenize a configuration file. */
typedef struct {
   /* File information: */
   const char *fileName;
   const Str *fileContents;
   /* Token handling: */
   ConfTokenState tokenState;
   ConfTokenPosition symbolPosition;
   unsigned int lineNumber;
   /* Debugging: */
   unsigned int totalWarnings;
} ConfScannerClient;

typedef enum {
   TOKEN_KEYWORD = 1,
   TOKEN_ASSIGN,
   TOKEN_STRING,
   TOKEN_END
} ConfTokenType;

/* Token structure used for interacting between
   the scanner and the parser. */
typedef struct {
   ConfTokenType type;
   unsigned int start;
   unsigned int length;
} ConfToken;

/* Public prototypes: */
Bool ConfScannerInitClient( ConfScannerClient *client, const char *path );
void ConfScannerRemoveClient( ConfScannerClient *client );
Bool ConfScannerGetToken( ConfScannerClient *client, ConfToken *token );

#endif
