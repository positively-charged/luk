/*

   Here are a few useful string functions I made for C. To keep the function 
   names small, I shortened some words in the function names that have common
   abbreviations, like 'str' for 'string' and 'del' for 'delete.' Note that
   any function ending with a C indicates that the function takes in a
   C string as an argument. The Str creation functions also take a C string.

   - Positron

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

#ifndef STRUTIL_H
#define STRUTIL_H

#include <string.h>

#include "gentype.h"

/* Our own string structure for easier handling of strings. */
typedef struct {
   unsigned int length;
   char *value;
} Str;

/* Note that many of these functions return a brand new string allocated in
   memory, so it's vital that the user frees the string after use. One can
   use the StrDel() function to do so. */

/* Function to create an Str string from a given C string. */
Str *StrNew( const char *string );
/* Makes a new substring from given C string. */
Str *StrNewSub( const char *string, unsigned int subLength );
/* Function to create a dynamically allocated empty string, with the NULL
   character already appended. */
Str *StrNewEmpty( size_t length );

/* Similar to PHP's substr() function. */
Str *StrSub( const Str *string, size_t position, int length );

/* You can use free() to delete the strings that are passed from the functions
   in this library, but for consistency's sake, use this one which does
   exactly the same thing. */
void StrDel( const Str *string );

/* The trim functions remove white space from appropriate 
   places of the string. */
Str *StrTrimLeft( const Str *string );
Str *StrTrimRight( const Str *string );
Str *StrTrim( const Str *string );

/* The reduce function removes any subsequent whitespace characters
   that may follow a whitespace character. */
Str *StrReduce( const Str *string );

/* Case change. */
Str *StrDown( const Str *string );
Str *StrUp( const Str *string );

/* Copy string. */
Str *StrCopy( const Str *string );

/* This function is similar to strpos() from PHP, returning the position
   of the first instance of a given character.
   TODO: add offset support */
int StrPos( const Str *string, const char targetCharacter );

/* Function to concatenate to Str strings. */
Str *StrConcat( const Str *first, const Str *second  );

/* Comparison function, similar to strcmp() == 0 */
Bool StrIsEqual( const Str *first, const Str *second );

#endif
