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
#include <time.h>

#include "strutil.h"

#include "print.h"

static void Print( const char *format, va_list *arguments ) {
   struct tm *timePieces;
   time_t timeStamp = time( NULL );

   timePieces = localtime( &timeStamp );
   printf( "[%02d:%02d:%02d] ", timePieces->tm_hour, timePieces->tm_min,
      timePieces->tm_sec );

   vprintf( format, *arguments );
   fflush( stdout );
}

void PrintMessage( const char *format, ... ) {
   va_list arguments;

   va_start( arguments, format );
   Print( format, &arguments );
   va_end( arguments );
}

void PrintLabel( const char *labelName, const char *format, va_list *args ) {
   Str *label = StrNew( labelName );
   Str *formatString = StrNew( format );
   Str *messageString = StrConcat( label, formatString );

   Print( messageString->value, args );

   StrDel( label );
   StrDel( formatString );
   StrDel( messageString );
}

void PrintNotice( const char *format, ... ) {
   va_list arguments;

   va_start( arguments, format );
   PrintLabel( "Notice: ", format, &arguments );
   va_end( arguments );
}

void PrintWarning( const char *format, ... ) {
   va_list arguments;

   va_start( arguments, format );
   PrintLabel( "Warning: ", format, &arguments );
   va_end( arguments );
}

void PrintError( const char *format, ... ) {
   va_list arguments;

   va_start( arguments, format );
   PrintLabel( "Error: ", format, &arguments );
   va_end( arguments );
}

void PrintHeader( const char *title ) {
   PrintMessage( "----- %s -----\n", title );
}

void PrintConfError( const char *const errorMsg ) {
   va_list noArguments;
   Print( errorMsg, &noArguments );
   printf( "\n" );
}
