/*

   Functions to parse a very simple configuration file similar
   in syntax to an INI file.

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

#ifndef CONF_H
#define CONF_H

#include <string.h>

#include "gentype.h"
#include "strutil.h"

#include "conf_scanner.h"

/* For now, we are going to put a static limit on the maximum number
   of parameters allowed to be read. */
#define CONF_MAX_PARAMETERS 32

/* Pointer to a function that will be called when a reading error occurs. */
typedef void ( *ConfReadErrViewer ) ( const char *const errorMsg );

typedef struct {
   Str *name;
   Str *value;
} ConfParameter;

typedef struct {
   ConfReadErrViewer errViewer;
   ConfScannerClient scanClient;
   ConfParameter params[ CONF_MAX_PARAMETERS ];
   int totalParams;
} Conf;

/* Function to load the configuration file into memory and prepare it for 
   reading. */
Bool ConfOpen( Conf *configFile, const char *path );
/* This function parses the data and prepares the parameters for further
   handling by the client. */
Bool ConfRead( Conf *configFile );
/* Function to get a parameter value. NULL is returned if the parameter 
   with the given name doesn't exist. */
const Str *ConfGetValue( const Conf *configFile, 
   const char *name );
void ConfClose( Conf *configFile );
void ConfSetReadErrorViewer( Conf *configFile, ConfReadErrViewer errViewer );
ConfReadErrViewer ConfGetReadErrorViewer( Conf *configFile );

#endif
