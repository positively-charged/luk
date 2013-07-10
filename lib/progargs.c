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

#include <string.h>
#include <stdio.h>

#include "progargs.h"

static char **list = NULL;
static char **arg = NULL;

void PROGA_Init( char **argv ) {
   /* We don't need the program path. */
   list = argv + 1;
   arg = list;
}

Bool PROGA_FindArg( const char *target ) {
   arg = list;

   while ( *arg != NULL && strcmp( *arg, target ) != 0 ) {
      arg += 1;
   }

   return ( *arg != NULL );
}

const char *PROGA_NextArg( void ) {
   if ( arg != NULL ) {
      arg += 1;
   }

   return PROGA_GetArg();
}

const char *PROGA_GetArg( void ) {
   return *arg;
}
