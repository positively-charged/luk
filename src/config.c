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

#include "strutil.h"

#include "print.h"
#include "config.h"

/* Private functions to enhance readability. */
static ConfigParameter *ConfigGetFirstParameter( void );
static ConfigParameter *ConfigGetNextParameter( ConfigParameter *parameter );

/* Our little parameter database. */
static ConfigParameter parameters[] = {
   { "server_address", NULL, TRUE },
   { "server_port", NULL, TRUE },
   { "server_password", NULL, TRUE },
   { "database_path", NULL, TRUE },
   { "database_save_on_store", NULL, FALSE },
   { NULL, NULL, FALSE },
};

Bool ConfigPopulate( const Conf *configFile ) {
   ConfigParameter *parameter = ConfigGetFirstParameter();
   const Str *value;

   while ( parameter->name != NULL ) {
      value = ConfGetValue( configFile, parameter->name );
      if ( value != NULL ) {
         parameter->value = StrCopy( value );
      }
      else {
         /* We need to check if the missing parameter is required. If
            so, then we abort the operation. */
         if ( parameter->isRequired ) {
            PrintError( "Missing required parameter in configuration "
               "file: %s\n", parameter->name );
            return FALSE;
         }
      }

      parameter = ConfigGetNextParameter( parameter );
   }

   return TRUE;
}

ConfigParameter *ConfigGetFirstParameter( void ) {
   return parameters;
}

ConfigParameter *ConfigGetNextParameter( ConfigParameter *parameter ) {
   return parameter + 1;
}

const Str *ConfigGetValue( const char *name ) {
   ConfigParameter *parameter = ConfigGetFirstParameter();
   while ( parameter->name != NULL ) {
      if ( strcmp( name, parameter->name ) == 0 ) {
         return parameter->value;
      }

      parameter = ConfigGetNextParameter( parameter );
   }

   return NULL;
}

void ConfigDisplay( void ) {
   ConfigParameter *parameter = ConfigGetFirstParameter();
   while ( parameter->name != NULL ) {
      const char *parameterValue;
      if ( parameter->value != NULL ) {
         parameterValue = parameter->value->value;
      }
      else {
         parameterValue = "(null)";
      }

      printf( "%s -> %s\n", parameter->name, parameterValue );
      parameter = ConfigGetNextParameter( parameter );
   }
}

void ConfigShutdown( void ) {
   ConfigParameter *parameter = ConfigGetFirstParameter();
   while ( parameter->name != NULL ) {
      StrDel( parameter->value );
      parameter->value = NULL;
      parameter = ConfigGetNextParameter( parameter );
   }
}
