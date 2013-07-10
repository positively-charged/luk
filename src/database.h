/*

   The database is in charge of saving data that the wad sends back to luk.
   The database uses a lukd file as the means for storage.

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

#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>

#include "gentype.h"
#include "strutil.h"

#include "luk.h"

/* Even though the lukd file specification sets no limits on the total 
   records a database file might have, we will set a limit on our database
   for our purposes. */
#define DATABASE_MAX_ENTRIES 1024
#define DATABASE_MAX_RECORDS 1024
#define DATABASE_RECORD_MAX_SIZE 1024

enum {
   DB_INIT_SUCCESS,
   DB_INIT_RECORDS_LOAD_FAILED,
   DB_INIT_FAILED
};

/* We're going to use a linked list for the records. */
typedef struct DatabaseRecord {
   Str *key;
   Str *value;
   struct DatabaseRecord *nextRecord;
} DatabaseRecord;

typedef struct DatabaseMapEntry {
   Str *name;
   struct DatabaseMapEntry *nextEntry;
   DatabaseRecord *firstRecord;
   unsigned int totalRecords;
} DatabaseMapEntry;

typedef struct {
   Bool isOperational;
   /* Entries: */
   DatabaseMapEntry *firstMap;
   DatabaseMapEntry *currentMap;
   unsigned int totalMaps;
   unsigned int totalRecords;
   unsigned int updatesSinceLastSave;
} Database;

/* This is the public interface, containing the functions to be used 
   for communicating with the database storage mechanism. */
void DatabaseInitialize( void );
int DatabaseInitializeFile( const char *pathToDatabaseFile );
/* This function tells the caller whether the database needs saving
   by checking if any updates were done to the database. */
Bool DatabaseIsSaveNeeded( void );
Bool DatabaseSave( const char *databaseOutPath );
void DatabaseShutdown( void );
const Str *DatabaseGetCurrentMap( void );
Bool DatabaseChangeMap( const Str *newCurrentMapName );
const Str *DatabaseRetrieve( const Str *name );
Bool DatabaseDelete( const Str *map );
/* This function either updates an existing record with the same key or 
   appends it as a new record if the key doesn't exist . */
void DatabaseStore( const Str *name, const Str *value );
int DatabaseCalculateRecordsTotalSize( void );
/* Debug functions */
void DatabasePrint( const Str *selectedMap );
void DatabasePrintMapEntry( const DatabaseMapEntry *entry );
void DatabasePrintRecord( const DatabaseRecord *record );

#endif
