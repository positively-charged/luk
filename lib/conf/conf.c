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
#include <stdlib.h>
#include <string.h>

#include "gentype.h"
#include "strutil.h"

#include "conf.h"
#include "conf_problem.h"
#include "conf_parser.h"

Bool ConfOpen( Conf *configFile, const char *path ) {
   configFile->totalParams = 0;
   configFile->errViewer = NULL;
   return ConfScannerInitClient( &configFile->scanClient, path );
}

Bool ConfRead( Conf *configFile ) {
   ConfProblemSetFile( configFile );
   return ConfParserLoadEntries( configFile, &configFile->scanClient );
}

const Str *ConfGetValue( const Conf *configFile, 
   const char *name ) {
   int entry;
   for ( entry = 0; entry < configFile->totalParams; entry += 1 ) {
      if ( strcmp( configFile->params[ entry ].name->value, name ) == 0 ) {
         return configFile->params[ entry ].value;
      }
   }

   return NULL;
}

void ConfClose( Conf *configFile ) {
   if ( configFile != NULL ) {
      /* Get rid of the scanner client: */
      ConfScannerRemoveClient( &configFile->scanClient );

      /* First, we need to get rid of the entries in the configuration file. */
      while ( configFile->totalParams > 0 ) {
         configFile->totalParams -= 1;
         StrDel( configFile->params[ configFile->totalParams ].name );
         StrDel( configFile->params[ configFile->totalParams ].value );
      }
   }
}

void ConfSetReadErrorViewer( Conf *configFile,
   ConfReadErrViewer errViewer ) {
   configFile->errViewer = errViewer;
}

ConfReadErrViewer ConfGetReadErrorViewer( Conf *configFile ) {
   return configFile->errViewer;
}
