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
#include <stdarg.h>

#include "conf_problem.h"
#include "conf_scanner.h"

/* Private prototypes: */
static const char *ConfProblemGetName( const ConfProblemType type );

/* We will hold on to the currently tested configuration file because
   it contaings important information that we will need when displaying
   the problem message. */
static Conf *testFile;

void ConfProblemSetFile( Conf *configFile ) {
   testFile = configFile;
}

void ConfProblemShow( const ConfProblemType probType, 
   const char *probMsgFormat, ... ) {
   /* For now, we will set a static limit on our problem messages and hope
      the problem messages will not go over this limit when used with sprintf(). 
      We should never make such assumptions, because they might lead to 
      problems like buffer overflows. For now, seeing how our error and
      warning messages cannot exceed this limit, this will do. But a better
      approach is warranted. */
   char probMsg[ CONF_PROBLEM_MAX_SIZE ];

   /* Append information about the location of the problem. */
   const char *probName = ConfProblemGetName( probType );
   int probMsgBodyOffset = sprintf( probMsg, "%s:%d: %s: ",
      testFile->scanClient.fileName, testFile->scanClient.lineNumber, 
      probName );

   /* Then append the actual error message. */
   ConfReadErrViewer probMsgWiewer = NULL;
   va_list probMsgArgs;

   va_start( probMsgArgs, probMsgFormat );
   vsprintf( probMsg + probMsgBodyOffset, probMsgFormat, probMsgArgs );
   va_end( probMsgArgs );

   probMsgWiewer = ConfGetReadErrorViewer( testFile );
   if ( probMsgWiewer != NULL ) {
      probMsgWiewer( probMsg );
   }
   else {
      printf( "%s\n", probMsg );
   }
}

const char *ConfProblemGetName( const ConfProblemType type ) {
   if ( type == CONF_PROBLEM_WARNING ) {
      return "warning";
   }
   else {
      return "error";
   }
}
