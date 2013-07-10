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

#include "query.h"
#include "print.h"

/* Private prototypes: */
static Str *QueryRemoveCapsule( const Str *capsule );
static Bool QueryIsValidPrefix( const char *queryPrefix );

/* Variable to hold data of the current query. */
static query_t query = { 0, NULL };

Bool QueryIsValidCapsule( const Str *capsule ) {
   /* Make sure the query capsule is of adequete length. */
   if ( capsule->length < QUERY_PREFIX_LENGTH ) {
      return FALSE;
   }

   /* We need to check whether the capsule is delimited by the appropriate
      delimiter character, a character that is invalid in player input. 
      NOTE: A better, more secure, mechanism is in need. */
   if ( capsule->value[ 0 ] == QUERY_DELIMITER &&
      capsule->value[ capsule->length - 1 ] == QUERY_DELIMITER ) {
      return TRUE;
   }
   else {
      return FALSE;
   }
}

Bool QueryUnpack( const Str *capsule ) {
   Bool isUnpacked = FALSE;

   /* First, remove the query capsule. */
   Str *rawQuery = QueryRemoveCapsule( capsule );

   Str *trimmedQuery = StrTrim( rawQuery );
   Str *cleanedQuery = StrReduce( trimmedQuery );

   size_t queryIdSize = 0;
   char queryIdString[ QUERY_ID_MAX_DIGITS + 1 ];

   const char *queryPos = cleanedQuery->value;
   Bool isIdInvalid = FALSE;

   /* First, get rid of the intermediate strings. */
   StrDel( rawQuery );
   StrDel( trimmedQuery );

   /* Proceed to check whether the prefix is valid. */
   if ( QueryIsValidPrefix( queryPos ) ) {
      queryPos += QUERY_PREFIX_LENGTH;
   }
   else {
      StrDel( cleanedQuery );
      return FALSE;
   }

   /* Collect the query ID. */
   isIdInvalid = FALSE;
   queryPos += 1;

   do {
      if ( isdigit( *queryPos ) && queryIdSize < QUERY_ID_MAX_DIGITS ) {
         queryIdString[ queryIdSize ] = *queryPos;
         queryIdSize += 1;
         queryPos += 1;
      }
      else {
         isIdInvalid = TRUE;
      }
   } while ( ! isspace( *queryPos ) && ! isIdInvalid );

   if ( isIdInvalid == FALSE ) {
      query_id_t newQueryId;

      queryIdString[ queryIdSize ] = '\0';
      newQueryId = atoi( queryIdString );

      /* If the query ID is valid and is higher than all previous IDs, 
         or the query is a debug query, proceed to get the cargo. */
      if ( newQueryId > QueryGetId() || newQueryId == 0 ) {
         query.id = newQueryId;

         queryPos += 1;
         query.cargo = StrNew( queryPos );

         isUnpacked = TRUE;
      }
      else {
         PrintWarning( 
            "Query with an older query ID received: new( %d ), old( %d )\n", 
            newQueryId, QueryGetId() );
      }
   }
   else {
      PrintNotice( "Invalid query ID given in received query\n" );
   }

   StrDel( cleanedQuery );
   return isUnpacked;
}

Str *QueryRemoveCapsule( const Str *capsule ) {
   return StrNewSub( capsule->value + 1, capsule->length - 2 );
}

Bool QueryIsValidPrefix( const char *queryPrefix ) {
   const char *prefix = QUERY_PREFIX;
   const size_t length = QUERY_PREFIX_LENGTH;
   size_t read = 0;

   while ( *queryPrefix != '\0' && tolower( *queryPrefix ) == *prefix &&
      read < length ) {
      prefix += 1;
      queryPrefix += 1;
      read += 1;
   }

   return ( read == length );
}

void QueryDeleteCargo( void ) {
   if ( query.cargo != NULL ) {
      StrDel( query.cargo );
   }
}

query_cargo_t QueryGetCargo( void ) {
   return query.cargo;
}

query_id_t QueryGetId( void ) {
   return query.id;
}

void QueryResetId( void ) {
   query.id = 0;
}
