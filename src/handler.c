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

#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "strutil.h"

#include "handler.h"
#include "command.h"
#include "database.h"
#include "print.h"

/* Private handler helpers prototypes: */
static int HandlerEncodeValueInAscii( const char *value, const int vLength );
static void HandlerEndStringTransmission( void );

/* Setup the structure that will help us with transferring strings. */
static string_transm_t st = { NULL, 0, 0, FALSE, 0 };

void HandlerStore( const command_t *command ) {
   if ( command->argsCount >= 2 ) {
      /* Record names should begin with a letter. */
      if ( isalpha( command->args[ 0 ]->value[ 0 ] ) ) {
         const Str *key = command->args[ 0 ];
         const Str *value = command->args[ 1 ];
         DatabaseStore( key, value );
         PrintMessage( "Storing \"%s\" in \"%s\"\n", value->value, key->value );
      }
      else {
         PrintNotice( "Record names should begin with a letter\n" );
      }
   }
   else {
      PrintNotice(
         "Missing arguments for STORE command. Dropping command\n" );
   }
}

void HandlerStoreDate( const command_t *command ) {
	if ( command->argsCount >= 1 ) {
      /* Because the database only supports the Str datatype as its storage
         mechanism, we need to convert the numeric timestamp into a string. */
      int timestamp = ( int ) time( 0 );
      Str *date;
      char stringDate[ 20 ];
      int dateLength = 0;
      int pos;

      /* Convert the numeric value into a reversed string. */
      while ( timestamp > 0 ) {
         stringDate[ dateLength ] = ( char ) ( '0' + timestamp % 10 );
         dateLength += 1;
         timestamp /= 10;
      }

      /* Now reverse the string into normal form. */
      date = StrNewEmpty( dateLength + 1 );
      pos = 0;

      while ( pos < dateLength ) {
         date->value[ pos ] = stringDate[ dateLength - pos - 1 ];
         pos += 1;
      }

      DatabaseStore( command->args[ 0 ], date );
      StrDel( date );
   }
   else {
		PrintNotice( "No date key was passed to STORE_DATE command\n" );
   }
}

void HandlerRetrieveDate( const command_t *command ) {
   if ( command->argsCount >= 1 ) {
      const Str *value = DatabaseRetrieve( command->args[ 0 ] );

      if ( value != NULL ) {
         time_t timestamp = ( time_t ) atoi( value->value );
         struct tm *date = localtime( &timestamp );
         int encodedDate = 0;

         /* We encode the date in ISO format ( YYYY-MM-DD ).
            We move the year 4 digits to the left by multiplying by 10000. */
         encodedDate = ( 1900 + date->tm_year ) * 10000;
         /* For the month, we want it two digits from the right. */
         encodedDate += ( date->tm_mon + 1 ) * 100;
         /* The day will occupy the first two smaller digits of the integer. */
         encodedDate += date->tm_mday;

         ReplySetDataInt( encodedDate );
         ReplySetResult( CMD_RETRIEVE_OK );
      }
      else {
         ReplySetDataInt( 0 );
         ReplySetResult( CMD_RETRIEVE_FAIL );
         PrintNotice( "Asked for a non-existant date record with key: %s\n",
            command->args[ 0 ]->value );
      }
   }
   else {
      PrintNotice( "Missing date key for RETRIEVE_DATE command\n" );
   }
}

void HandlerRetrieve( const command_t *command ) {
   if ( command->argsCount >= 1 ) {
      const Str *key = command->args[ 0 ];
      const Str *value = DatabaseRetrieve( key );

      if ( value != NULL ) {
         ReplySetDataStr( value );
         ReplySetResult( CMD_RETRIEVE_OK );
      }
      else {
         ReplySetDataInt( 0 );
         ReplySetResult( CMD_RETRIEVE_FAIL );
         PrintNotice( "Asked for a non-existant record with key: %s\n", 
            key->value );
      }
   }
   else {
      PrintNotice( "Missing key for retrieve command\n" );
   }
}

void HandlerRetrieveStringInit( const command_t *command ) {
   const Str *recordName;
   const Str *recordValue;
   int queriesNeeded;

   /* We return an if no argument is given. */
   if ( command->argsCount > 0 ) {
      recordName = command->args[ 0 ];
   }
   else {
      PrintNotice( 
         "Missing record key for string retrieval. Aborting operation\n" );
      return;
   }

   recordValue = DatabaseRetrieve( recordName );

   if ( recordValue != NULL ) {
      PrintMessage( "Starting string transmission for record: %s\n",
         recordName->value );

      /* If a previous transmission was not completed while another one is
         activated, we remove the previous one. */
      if ( st.isActive ) {
         PrintWarning( "Terminating active string transmission to start "
            " a new one\n" );
         HandlerEndStringTransmission();
      }

      queriesNeeded = recordValue->length / HANDLER_QUERY_MAX_CHARS;
      /* The last query might not be a full query, meaning it won't transfer
         the maximum possible characters allowed per query, but we still need
         an extra query to transfer the remaining data. */
      if ( recordValue->length % HANDLER_QUERY_MAX_CHARS != 0 ) {
         queriesNeeded += 1;
      }

      st.value = StrCopy( recordValue );
      st.queriesNeeded = queriesNeeded;
      st.isActive = TRUE;
      st.offset = 0;
      st.charsLeft = recordValue->length;

      ReplySetDataInt( queriesNeeded );
      ReplySetResult( CMD_RETRIEVE_OK );
   }
   else {
      PrintNotice( "Asked for a non-existant string record with key: %s\n", 
         recordName->value );
      ReplySetDataInt( 0 );
      ReplySetResult( CMD_RETRIEVE_FAIL );
   }
}

void HandlerRetrieveStringSegment( const command_t *command ) {
   /* Remove unsused parameter warning from strict compilers. */
   ( void ) command;

   /* Make sure we have an active transmission before proceeding. */
   if ( st.isActive ) {
      int asciiPackage;

      int segmentLength = HANDLER_QUERY_MAX_CHARS;
      if ( st.charsLeft < HANDLER_QUERY_MAX_CHARS ) {
         segmentLength = st.charsLeft;
      }

      asciiPackage = HandlerEncodeValueInAscii( 
         st.value->value + st.offset, segmentLength );
      st.offset += segmentLength;
      st.charsLeft -= segmentLength;

      ReplySetDataInt( asciiPackage );
      ReplySetResult( CMD_RETRIEVE_OK );
      PrintMessage( "Sending string segment: %d\n", asciiPackage );

      /* End transmission when all segments have been sent. */
      st.queriesNeeded -= 1;
      if ( st.queriesNeeded <= 0 ) {
         HandlerEndStringTransmission();
      }
   }
   else {
      ReplySetDataInt( 0 );
      ReplySetResult( CMD_RETRIEVE_FAIL );
      PrintNotice( 
         "A string transmission is not open. Failed to get segment\n" );
   }
}

void HandlerEndStringTransmission( void ) {
   if ( st.isActive ) {
      PrintMessage( "Closing string transmission\n" );
      StrDel( st.value );
      st.isActive = FALSE;
   }
}

int HandlerEncodeValueInAscii( const char *value, const int vLength ) {
   int asciiPackage = 0;
   int position;
   int separator = 1;

   for ( position = vLength - 1; position >= 0; position -= 1 ) {
      asciiPackage += ( ( int ) value[ position ] + HANDLER_ASCII_PADDING ) * 
         separator;
      separator *= 1000; 
   }

   return asciiPackage;
}

/* Exit function for all handlers, just in case any handlers need to
   do something before program exit. */
void HandlerExit( void ) {
   /* Close any string tranmission that might be active. */
   HandlerEndStringTransmission();
}

void HandlerPrint( const command_t *command ) {
   if ( command->argsCount > 0 ) {
      PrintMessage( "%s\n", command->args[ 0 ]->value );
   }
}

void HandlerPrintDatabase( const command_t *command ) {
   const Str *map = NULL;

   if ( command->argsCount > 0 ) {
      map = command->args[ 0 ];
   }

   DatabasePrint( map );
}
