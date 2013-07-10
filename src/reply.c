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
#include <string.h>

#include "reply.h"

/* This private variable will hold information about a query reply. */
static reply_t reply;

void ReplyReset( void ) {
   reply.queryId = 0;
   reply.queryResult = 0;
   reply.data[ 0 ] = '\0';
   reply.dataSize = 0;
}

void ReplySetQueryId( query_id_t id ) {
   reply.queryId = id;
}

void ReplySetDataStr( const Str *value ) {
   size_t length = value->length;
   if ( length > REPLY_DATA_MAX_CHARACTERS ) {
      length = REPLY_DATA_MAX_CHARACTERS;
   }

   memcpy( reply.data, value->value, length );
   reply.data[ length ] = '\0';
   reply.dataSize = length;
}

void ReplySetDataInt( int value ) {
   /* Since we set the size of the data buffer to hold the maximum int
      value in string form, we should not worry about a buffer overflow. */
   reply.dataSize = sprintf( reply.data, "%d", value );
}

void ReplySetResult( size_t result ) {
   reply.queryResult = result;
}

size_t ReplyGetDataSize( void ) {
   return reply.dataSize;
}

Str *ReplyBuildCommand( void ) {
   Str *replyCommand;

   const size_t replyCommandSize = REPLY_COMMAND_LAYOUT_LENGTH + 
      QUERY_ID_MAX_DIGITS + reply.dataSize + 1;
   replyCommand = StrNewEmpty( replyCommandSize );

   sprintf( replyCommand->value, REPLY_COMMAND_LAYOUT, reply.data, 
      reply.queryId, ( int ) reply.queryResult );

   return replyCommand;
}
