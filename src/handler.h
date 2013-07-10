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

#ifndef HANDLER_H
#define HANDLER_H

#include "reply.h"
#include "command.h"

#define HANDLER_QUERY_MAX_CHARS 3
/* The padding is used to make any ASCII value equal three digits
   in length for easier handling. */
#define HANDLER_ASCII_PADDING 100
 
/* Structure for storing all necessary data for string retrieval. */
typedef struct {
   const Str *value;
   int queriesNeeded;
   int offset;
   Bool isActive;
   size_t charsLeft;
} string_transm_t;

/* RETRIEVE queries results. */
typedef enum {
   CMD_RETRIEVE_OK,
   CMD_RETRIEVE_FAIL
} cmd_retrieve_result_t;

/* Command handlers and their helpers: */
void HandlerRetrieve( const command_t *command );
void HandlerRetrieveDate( const command_t *command );
void HandlerRetrieveStringInit( const command_t *command );
void HandlerRetrieveStringSegment( const command_t *command );
void HandlerStore( const command_t *command );
void HandlerStoreDate( const command_t *command );
void HandlerPrint( const command_t *command );
void HandlerPrintDatabase( const command_t *command );
void HandlerExit( void );

#endif
