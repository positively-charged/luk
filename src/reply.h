/*

   The reply file helps us with handling replies to luk queries.

   Reply command structure:
      <data>; <query_id>; <query_execution_result>

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

#ifndef REPLY_H
#define REPLY_H

#include <stdlib.h>

#include "strutil.h"

#include "query.h"

/* To keep the console output as short as possible, we are going to shorten
   the console variables as much as possible but still keep them meaningful. */
#define REPLY_COMMAND_LAYOUT \
   "set luk_d \"%s\"; set luk_qid \"%d\"; set luk_qr \"%d\""
#define REPLY_COMMAND_LAYOUT_LENGTH 49

/* Since the amount of data we can send to one console variable in one go
   is limited to an integer value, we will set a static limit on the maximum
   data size for one reply. */
#define REPLY_DATA_MAX_CHARACTERS 10

typedef struct {
   query_id_t queryId;
   size_t queryResult;
   char data[ REPLY_DATA_MAX_CHARACTERS ];
   size_t dataSize;
} reply_t;

void ReplyReset( void );
void ReplySetQueryId( query_id_t id );
void ReplySetDataStr( const Str *value );
void ReplySetDataInt( int value );
void ReplySetResult( size_t result );
size_t ReplyGetDataSize( void );
Str *ReplyBuildCommand( void );

#endif
