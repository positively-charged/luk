/*

   This file handles luk queries sent from the wad.

   --------------------------------------------------------------------------

   Structure of a luk query:

   Query Capsule:       left_delimiter <query> right_delimiter
   Query:               identifier query_id <query_cargo>
   Query Cargo:         action [ [{]arg[}] [ ... ] ]

   The query capsule holds the query. It is needed to differentiate a luk query
   from the other server output. Right now, we use two delimiting characters
   on both ends of the query to produce the capsule. These delimiting 
   characters MUST NOT be characters that a player can inject into the server
   output for security reasons.

   Inside the query capsule is the query. The first field of the query is 
   the identifer. It is used as extra security to differentiate a luk query
   from the rest of the server output. Then follows the query ID. The query ID
   is a numeric value that is incremented with each query sent. It is used
   to reduce duplicate queries and, most importantly, to indicate to the 
   waiting wad when luk finishes processing the given query by sending back
   a reply that contains the query's query ID. The smallest query ID must be 1.
   Queries with a query ID of 0 are debug queries, used for the purpose of
   debugging. Finally comes the query cargo.

   The query cargo is also known as a command. It contains the actual data
   that will be processed by luk's command handlers.

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

#ifndef QUERY_H
#define QUERY_H

#include "gentype.h"
#include "strutil.h"

/* The prefix must be in lowercase for the query check to work. */
#define QUERY_PREFIX "luk"
#define QUERY_PREFIX_LENGTH 3
#define QUERY_DELIMITER '\b'

/* The number of digits that a query ID can have. We will set it to the
   number of digits that the maximum unsigned int value must have. */ 
#define QUERY_ID_MAX_DIGITS 9

typedef unsigned int query_id_t;
typedef const Str * query_cargo_t;

typedef struct {
   query_id_t id;
   query_cargo_t cargo;
} query_t;

Bool QueryIsValidCapsule( const Str *capsule );
Bool QueryUnpack( const Str *line );
query_id_t QueryGetId( void );
query_cargo_t QueryGetCargo( void );
void QueryDeleteCargo( void );
void QueryResetId( void );

#endif
