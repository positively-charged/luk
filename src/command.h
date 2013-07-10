/*

   This file takes care of checking and executing luk commands.

   luk command:
      '<action> [ [{]<argument>[}] [ ... ] ]'

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

#ifndef COMMAND_H
#define COMMAND_H

#include "gentype.h"
#include "strutil.h"

#define LUK_PRINT_ERROR_MESSAGES FALSE
#define LUK_PRINT_MESSAGES TRUE

/* We will put a static argument limit for now. */
#define LUK_COMMAND_MAXIMUM_ARGUMENTS 5

typedef struct command_struct_t {
   void ( *handler ) ( const struct command_struct_t *command );
   const Str *args[ LUK_COMMAND_MAXIMUM_ARGUMENTS ];
   int argsCount;
} command_t;

/* This struct is used for convenience to centralize the name and handler
   of a command. */
typedef struct {
   const char *action;
   void ( *handler ) ( const command_t *command );
} command_info_t;

command_t *CommandCreate( const Str *commandData );
void CommandExecute( const command_t *command );
void CommandDestroy( command_t *command );

#endif
