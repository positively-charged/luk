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
#include <stdlib.h>
#include <ctype.h>

#include "luk.h"
#include "reply.h"
#include "command.h"
#include "handler.h"
#include "print.h"

/* Nice little database to organize available commands. The command names
   should be in uppercase for the functions that use this database to work
   properly, reducing unnecessary uppercase function calls in the process. */
static const command_info_t commandsDatabase[] = {
   { "STORE", HandlerStore },
   { "STORE_DATE", HandlerStoreDate },
   { "RETRIEVE", HandlerRetrieve },
   { "RETRIEVE_DATE", HandlerRetrieveDate },
   { "RETRIEVE_STRING_INITIATE", HandlerRetrieveStringInit },
   { "RETRIEVE_STRING_SEGMENT", HandlerRetrieveStringSegment },
   { "PRINT_DATABASE", HandlerPrintDatabase },
   { "PRINT", HandlerPrint },
   /* This statement must be present and should be the last statement
      in this database because it indicates the end of the database
      to the functions that use it. */
   { NULL, NULL },
};

/* Private prototypes: */
void CommandBuildArguments( command_t *command, const char *argData, 
   const Str *action );
const command_info_t *CommandGetInfo( const Str *name );

command_t *CommandCreate( const Str *commandData ) {
   command_t *command = NULL; 
   const command_info_t *commandInfo;
   const char *commandPos = commandData->value;

   Str *action;
   const char *actionStart = commandPos;
   size_t actionLength = 0;

   /* Actions can only have letters and underscores. */
   while ( isalpha( *commandPos ) || *commandPos == '_' ) {
      commandPos += 1;
      actionLength += 1;
   }

   action = StrNewSub( actionStart, actionLength );
   commandInfo = CommandGetInfo( action );

   if ( commandInfo->action != NULL ) {
      command = ( command_t * ) malloc( sizeof( command_t ) );
      if ( command != NULL ) {
         command->handler = commandInfo->handler;
         command->argsCount = 0;
         CommandBuildArguments( command, commandPos, action );
      }
   }
   /* If we have no way of handling a given action, we discard it. */
   else {
      PrintNotice( "Unknown action: %s. Discarding...\n", 
         action->value );
   }

   StrDel( action );

   return command;
}

const command_info_t *CommandGetInfo( const Str *action ) {
   const Str *targetAction = StrUp( action );
   const command_info_t *commandInfo = commandsDatabase;

   while ( commandInfo->action != NULL && 
      strcmp( targetAction->value, commandInfo->action ) != 0 ) {
      commandInfo += 1;
   }

   StrDel( targetAction );
   return commandInfo;
}

void CommandBuildArguments( command_t *command, const char *argData, 
   const Str *action ) {
   const char *argPos = argData;
   const char *argumentStart;
   int argumentLength;

   while ( *argPos != '\0' ) {
      if ( ! isspace( *argPos ) ) {
         /* Brace arguments: */
         if ( *argPos == '{' ) {
            argPos += 1;

            argumentStart = argPos;
            argumentLength = 0;

            while ( *argPos != '}' && *argPos != '\0' ) {
               argPos += 1;
               argumentLength += 1;
            }

            /* Only continue to the next position in the command data if
               we haven't reached the end already. */
            if ( *argPos != '\0' ) {
               argPos += 1;
            }
            /* Flash a warning if the brace argument was not 
               properly closed. */
            else {
               PrintWarning( "Brace argument for statement %s was not closed "
                  "properly\n", action->value );
            }
         }
         /* Space-terminated arguments: */
         else {
            argumentStart = argPos;
            argumentLength = 0;

            while ( ! isspace( *argPos ) && *argPos != '\0' ) {
               argPos += 1;
               argumentLength += 1;
            }
         }

         /* Only add the argument if the maximum arguments have not yet
            been reached. Otherwise, break out of the loop. */
         if ( command->argsCount < LUK_COMMAND_MAXIMUM_ARGUMENTS ) {
            command->args[ command->argsCount ] =
               StrNewSub( argumentStart, argumentLength );
            command->argsCount += 1;
         }
         else {
            PrintWarning( "Maximum arguments (%d) reached for command: %s."
               " Skiping the rest...\n", LUK_COMMAND_MAXIMUM_ARGUMENTS,
               action->value );
            break;
         }
      }
      else {
         argPos += 1;
      }
   }
}

void CommandExecute( const command_t *command ) {
   command->handler( command );
}

void CommandDestroy( command_t *command ) {
   if ( command != NULL ) {
      int arg;

      for ( arg = 0; arg < command->argsCount; arg += 1 ) {
         StrDel( command->args[ arg ] );
      }

      free( command );
   }
}
