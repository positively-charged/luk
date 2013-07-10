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
#include <ctype.h>

#include "strutil.h"

Str *StrNew( const char *string ) {
   return StrNewSub( string, strlen( string ) );
}

Str *StrNewSub( const char *string, unsigned int subLength ) {
   size_t length = strlen( string );
   Str *subString = NULL;

   /* Make sure the substring length isn't greater than the original
      string length. */
   if ( subLength > length ) {
      subLength = length;
   }

   if ( ( subString = StrNewEmpty( subLength ) ) != NULL ) {
      memcpy( subString->value, string, subString->length );
   }

   return subString;
}

Str *StrNewEmpty( size_t length ) {
   /* Allocate the necessary space to hold the Str struct and the amount
      of data given by length; return NULL on failure. */
   const size_t strSize = sizeof( Str );
   Str *emptyString = ( Str * ) malloc( strSize + length + 1 );

   if ( emptyString != NULL ) {
      emptyString->length = length;
      /* Point the Str value pointer to the fresh block of space to be
         used for storing the string data located right after the Str struct
         data fields. */
      emptyString->value = ( char * ) emptyString + strSize;
      /* Remove the potential garbage data. */
      memset( emptyString->value, '\0', emptyString->length + 1 );
   }

   return emptyString;
}

Str *StrSub( const Str *string, size_t position, int length ) {
   int substringLength;
   Str *substring;

   /* Any length below zero is an invalid length. Zero is valid 
      because it's used to indicate no length restriction, 
      meaning up to the end of the given string. */
   if ( string == NULL || length < 0 ) {
      return NULL;
   }

   if ( length == 0 ) {
      substringLength = string->length - position; 
   }
   else if ( position + length <= string->length ) {
      substringLength = length;
   }
   /* If the length from the given position exceeds the maximum
      length of the string, then we just count to the end of
      the string. */
   else {
      substringLength = string->length - position; 
   }

   substring = StrNewEmpty( substringLength );
   if ( substring != NULL ) {
      memcpy( substring->value, string->value + position, substring->length );
      return substring;
   }
   else {
      return NULL;
   }
}

void StrDel( const Str *string ) {
   if ( string != NULL ) {
      free( ( void * ) string );
   }
}

Str *StrTrimLeft( const Str *string ) {
   unsigned int whitespaceRead;

   /* Return with an error if the string is invalid: */
   if ( string == NULL ) {
      return NULL;
   }

   whitespaceRead = 0;
   while ( whitespaceRead < string->length && 
      isspace( string->value[ whitespaceRead ] ) ) {
      whitespaceRead += 1;
   }

   return StrSub( string, whitespaceRead, 0 );
}

Str *StrTrimRight( const Str *string ) {
   unsigned int whitespaceRead;

   if ( string == NULL ) {
      return NULL;
   }

   whitespaceRead = 0;
   while ( whitespaceRead < string->length && 
      isspace( string->value[ string->length - whitespaceRead - 1 ] ) ) {
      whitespaceRead += 1;
   }

   return StrSub( string, 0, string->length - whitespaceRead );
}

Str *StrTrim( const Str *string ) {
   Str *leftTrimmedString;
   Str *fullyTrimmedString;

   if ( string == NULL ) {
      return NULL;
   }

   leftTrimmedString = StrTrimLeft( string );
   fullyTrimmedString = StrTrimRight( leftTrimmedString );
   StrDel( leftTrimmedString );
   return fullyTrimmedString;
}

Str *StrReduce( const Str *string ) {
   Str *cleanedString;
   size_t cleanedStringLength = 0;
   Str *reducedString;

   unsigned int character;
   char previousCharacter = '\0';
   char currentCharacter;

   if ( string == NULL ) {
      return NULL;
   }

   cleanedString = StrNewEmpty( string->length );
   if ( cleanedString == NULL ) {
      return NULL;
   }

   /* Reduce the whitespace in the string: */
   for ( character = 0; character < string->length; character += 1 ) {
      currentCharacter = string->value[ character ];
      if ( ( isspace( currentCharacter ) && ! isspace( previousCharacter ) ) ||
         ! isspace( currentCharacter ) ) {
         cleanedString->value[ cleanedStringLength ] = currentCharacter;
         cleanedStringLength += 1;
      }
      previousCharacter = currentCharacter;
   }

   /* Copy the string to an exact buffer size as the string length: */
   reducedString = StrNewEmpty( cleanedStringLength );
   if ( reducedString == NULL ) {
      return NULL;
   }
   memcpy( reducedString->value, cleanedString->value, reducedString->length );
   StrDel( cleanedString );

   return reducedString;
}

Str *StrDown( const Str *string ) {
   unsigned int character;
   Str *downString;

   if ( string == NULL ) {
      return NULL;
   }

   downString = StrNewEmpty( string->length );
   if ( downString == NULL ) {
      return NULL;
   }

   for ( character = 0; character < string->length; character += 1 ) {
      downString->value[ character ] = tolower( string->value[ character ] );
   }

   return downString;
}

Str *StrUp( const Str *string ) {
   unsigned int character;
   Str *upString;

   if ( string == NULL ) {
      return NULL;
   }

   upString = StrNewEmpty( string->length );
   if ( upString == NULL ) {
      return NULL;
   }

   for ( character = 0; character < string->length; character += 1 ) {
      upString->value[ character ] = toupper( string->value[ character ] );
   }

   return upString;
}

Str *StrCopy( const Str *string ) {
   Str *copyString = StrNewEmpty( string->length );
   memcpy( copyString->value, string->value, copyString->length );
   return copyString;
}

int StrPos( const Str *string, const char targetCharacter ) {
   unsigned int character;

   if ( string == NULL ) {
      return -2;
   }

   for ( character = 0; character < string->length; character+= 1 ) {
      if ( string->value[ character ] == targetCharacter ) {
         return character;
      }
   }

   return -1;
}

Str *StrConcat( const Str *first, const Str *second  ) {
   Str *fullString = StrNewEmpty( first->length + second->length );

   memcpy( fullString->value, first->value, first->length );
   memcpy( fullString->value + first->length, second->value, second->length );

   return fullString;
}

Bool StrIsEqual( const Str *first, const Str *second ) {
   if ( strcmp( first->value, second->value ) == 0 ) {
      return TRUE;
   }
   else {
      return FALSE;
   }
}
