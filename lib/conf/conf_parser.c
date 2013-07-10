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

#include "gentype.h"
#include "strutil.h"

#include "conf.h"
#include "conf_problem.h"
#include "conf_parser.h"
#include "conf_scanner.h"

/* Private prototypes: */
static ConfToken currentToken;
static void ConfParserGetNextToken( void );

static void ConfParserParseStatement( void );
static void ConfParserParseStatementTail( void );

static Bool isErrorTriggered = FALSE;
static void ConfParserTriggerError( void );
static Bool ConfParserIsErrorTriggered( void );
/* This function returns a name substitute for one of the token symbols
   above. */
const char *ConfParserGetTokenTypeName( const ConfTokenType type );

static Bool ConfParserMatch( const ConfTokenType tokenType );
static void ConfParserMatchIgnore( const ConfTokenType tokenType );
static void ConfParserMatchAppend( const ConfTokenType tokenType );

#define RECORD_PIECES_BUFFER_SIZE 2 /* Key / Value */
static ConfToken recordPieces[ RECORD_PIECES_BUFFER_SIZE ];
static int recordPiecesPosition = 0;
static void ConfParserAppendToken( void );
static void ConfParserAppendRecord( void );

/* Hold on to important values that are needed for parsing. */
static ConfScannerClient *scannerClient;
static Conf *configFile;

Bool ConfParserLoadEntries( Conf *file, ConfScannerClient *client ) {
   configFile = file;
   scannerClient = client;

   /* Begin parsing the configuration file. */
   ConfParserGetNextToken();
   ConfParserParseStatement();
   ConfParserParseStatementTail();

   if ( ! ConfParserIsErrorTriggered() ) {
      return TRUE;
   }
   else {
      return FALSE;
   }
}

void ConfParserGetNextToken( void ) {
   Bool isTokenGrabbed = ConfScannerGetToken( scannerClient, &currentToken );
   /* Trigger an error if the scanner has encountered an error of its own. */
   if ( isTokenGrabbed == FALSE ) {
      ConfParserTriggerError();
   }
}

void ConfParserParseStatement( void ) {
   if ( currentToken.type != TOKEN_END ) {
      ConfParserMatchAppend( TOKEN_KEYWORD );
      ConfParserMatchIgnore( TOKEN_ASSIGN );
      ConfParserMatchAppend( TOKEN_STRING );
      ConfParserAppendRecord();
   }
}

void ConfParserParseStatementTail( void ) {
   if ( currentToken.type != TOKEN_END && ! ConfParserIsErrorTriggered() ) {
      ConfParserParseStatement();
      ConfParserParseStatementTail();
   }
}

/* These are the different match functions that the parser will use to
   compare the tokens from the scanner against a predefined structure of
   tokens specified by the parser. */

Bool ConfParserMatch( const ConfTokenType tokenType ) {
   if ( ConfParserIsErrorTriggered() ) {
      return FALSE;
   }

   if ( tokenType == currentToken.type ) {
      return TRUE;
   }
   else {
      ConfProblemShow( 
         CONF_PROBLEM_ERROR,
         "Invalid statement. Expecting %s, but found %s instead.",
         ConfParserGetTokenTypeName( tokenType ), 
         ConfParserGetTokenTypeName( currentToken.type )
      );
      ConfParserTriggerError();
      return FALSE;
   }
}

void ConfParserMatchIgnore( const ConfTokenType tokenType ) {
   if ( ConfParserMatch( tokenType ) ) {
      ConfParserGetNextToken();
   }
}

void ConfParserMatchAppend( const ConfTokenType tokenType ) {
   if ( ConfParserMatch( tokenType ) ) {
      ConfParserAppendToken();
      ConfParserGetNextToken();
   }
}

void ConfParserAppendToken( void ) {
   recordPieces[ recordPiecesPosition ] = currentToken;
   recordPiecesPosition += 1;
   if ( recordPiecesPosition >= RECORD_PIECES_BUFFER_SIZE ) {
      recordPiecesPosition = 0;
   }
}

void ConfParserAppendRecord( void ) {
   if ( ConfParserIsErrorTriggered() ) {
      return;
   }

   /* Only append upto the maximum allowed. */
   if ( configFile->totalParams < CONF_MAX_PARAMETERS ) {
      ConfToken *name = &recordPieces[ 0 ];
      ConfToken *value = &recordPieces[ 1 ];

      ConfParameter param;
      param.name = 
         StrSub( scannerClient->fileContents, name->start, name->length );
      param.value = 
         StrSub( scannerClient->fileContents, value->start, value->length );

      configFile->params[ configFile->totalParams ] = param;
      configFile->totalParams += 1;
   }
}

void ConfParserTriggerError( void ) {
   isErrorTriggered = TRUE;
}

Bool ConfParserIsErrorTriggered( void ) {
   return isErrorTriggered;
}

const char *ConfParserGetTokenTypeName( const ConfTokenType type ) {
   switch ( type ) {
      case TOKEN_KEYWORD: return "a keyword";
      case TOKEN_ASSIGN: return "'='";
      case TOKEN_STRING: return "a string";
      case TOKEN_END: return "EOF";
      default: return "Unknown";
   }
}
