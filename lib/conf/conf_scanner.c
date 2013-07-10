/*

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "gentype.h"
#include "fileutil.h"

#include "conf.h"
#include "conf_problem.h"
#include "conf_scanner.h"

Bool ConfScannerInitClient( ConfScannerClient *client,
   const char *path ) {
   int fileSize;
   Str *fileData;

   FILE *file = fopen( path, "rb" );
   /* We abort if we cannot open the configuration file. */
   if ( file == NULL ) {
      return FALSE;
   }

   /* Find size: */
   fseek( file, 0, SEEK_END );
   fileSize = ftell( file );
   rewind( file );

   /* Load file data: */
   fileData = StrNewEmpty( fileSize );
   if ( fileData == NULL ) {
      return FALSE;
   }
   fread( fileData->value, 1, fileData->length, file );

   fclose( file );

   /* Initialize scanner client fields: */
   client->fileName = FileBasename( path );
   client->fileContents = fileData;
   client->lineNumber = 1;
   client->symbolPosition = 0;
   client->tokenState = STATE_START;
   client->totalWarnings = 0;

   return TRUE;
}

void ConfScannerRemoveClient( ConfScannerClient *client ) {
   if ( client != NULL ) {
      StrDel( client->fileContents );
   }
}

Bool ConfScannerGetToken( ConfScannerClient *client, 
   ConfToken *token ) {
   ConfTokenState state = client->tokenState;
   ConfTokenPosition position = client->symbolPosition;

   int stringTokenStartLine;

   /* Token: */
   ConfTokenType type;
   unsigned int start = 0;
   int length;
   Bool isTokenGrabbed = TRUE;
   Bool tokenReady = FALSE;
   ConfTokenPosition tokenStart = position;

   char symbol;
   int adjust;

   /* We set the token status to success by default: */
   isTokenGrabbed = TRUE;

   while ( TRUE ) {
      if ( position >= client->fileContents->length ) {
         type = TOKEN_END;
         tokenReady = TRUE;
         break;
      }

      symbol = client->fileContents->value[ position ];

      adjust = 1;

      switch ( state ) {
         case STATE_START:
            if ( isalpha( symbol ) ) {
               state = STATE_KEYWORD;
               tokenStart = position;
               start = position;
            }
            else if ( symbol == CONF_VALUE_KEY_SEPARATOR ) {
               type = TOKEN_ASSIGN;
               tokenReady = TRUE;
               length = 1;
               start = position;
            }
            else if ( symbol == CONF_STRING_DELIM ) {
               stringTokenStartLine = client->lineNumber;
               state = STATE_STRING;
               tokenStart = position + 1;
               start = position + 1;
            }
            else if ( symbol == CONF_COMMEND_START ) {
               state = STATE_COMMENT;
            }
            /* We skip whitespace that is not part of anything important,
               like a string. */
            else if ( isspace( symbol ) ) {
               break;
            }
            /* Invalid characters: */
            else {
               ConfProblemShow( 
                  CONF_PROBLEM_WARNING, 
                  "Invalid character spotted: '%c'", symbol 
               );
            }

            break;

         case STATE_KEYWORD:
            if ( ! ( isalnum( symbol ) || symbol == '_' ) ) {
               type = TOKEN_KEYWORD;
               tokenReady = TRUE;
               state = STATE_START;
               length = position - tokenStart;
               adjust = 0;
            }

            break;

         case STATE_STRING:
            if ( symbol == CONF_STRING_DELIM ) {
               type = TOKEN_STRING;
               tokenReady = TRUE;
               state = STATE_START;
               length = position - tokenStart;
            }

            break;

         case STATE_COMMENT:
            if ( symbol == CONF_COMMENT_END ) {
               state = STATE_START;
            }

            break;
      }

      position += adjust;

      if ( adjust == 1 && symbol == CONF_LINE_SEPARATOR ) {
         client->lineNumber += 1;
      }

      if ( tokenReady ) {
         break;
      }
   }

   if ( state == STATE_STRING ) {
      ConfProblemShow(
         CONF_PROBLEM_ERROR, 
         "String starting on line %d was not closed.",
         stringTokenStartLine );
      isTokenGrabbed = FALSE;
   }

   token->type = type;
   token->length = length;
   token->start = start;

   client->tokenState = state;
   client->symbolPosition = position;

   return isTokenGrabbed;
}
