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

#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "lukd.h"
#include "print.h"

/* Private functions */
static void DatabaseAppendMapEntry( DatabaseMapEntry *entry );
static DatabaseMapEntry *DatabaseCreateMapEntry( const Str *mapName );
static Bool DatabaseAppendRecord( DatabaseRecord *record );
static void DatabaseDestroyMapEntry( DatabaseMapEntry *entry );

/* Database variable: */
static Database database;

void DatabaseInitialize( void ) {
   database.firstMap = NULL;
   database.currentMap = NULL;
   database.totalMaps = 0;
   database.totalRecords = 0;
   database.isOperational = TRUE;
   database.updatesSinceLastSave = 0;
}

int DatabaseInitializeFile( const char *pathToStorage ) {
   Bool isImported;

   /* Initialize the necessary fields. */
   DatabaseInitialize();

   /* Get previous records from database file. */
   isImported = LukdImportDatabase( pathToStorage );
   /* After the previous records are imported, reset the updates counter
      back to zero because we loaded data that is already in the file, not
      new. */
   database.updatesSinceLastSave = 0;

   if ( isImported ) {
      return DB_INIT_SUCCESS;
   }
   else {
      return DB_INIT_RECORDS_LOAD_FAILED;
   }
}

Bool DatabaseIsSaveNeeded( void ) {
   return ( database.updatesSinceLastSave > 0 );
}

Bool DatabaseChangeMap( const Str *newCurrentMapName ) {
   Str *mapName = StrDown( newCurrentMapName );
   DatabaseMapEntry *newMapEntry;

   /* If the database has no map entries, we create a new entry for
      this map and set it as the current map. */
   if ( database.totalMaps == 0 ) {
      newMapEntry = DatabaseCreateMapEntry( mapName );
      DatabaseAppendMapEntry( newMapEntry );
   }
   /* Bail out if the new map name is the same name as that
      of the current map. */
   else if ( StrIsEqual( mapName, database.currentMap->name ) ) {
      StrDel( mapName );
      return FALSE;
   }
   /* Otherwise, search the existing map series. */
   else {
      newMapEntry = database.firstMap;
      while ( newMapEntry != NULL && 
         ! StrIsEqual( newMapEntry->name, mapName ) ) {
         newMapEntry = newMapEntry->nextEntry;
      }

      /* If the search is successful at locating an entry with the same name
         as the one given, make the entry the current entry. Otherwise, make
         a new entry with the given name and then make it current. */
      if ( newMapEntry == NULL ) {
         newMapEntry = DatabaseCreateMapEntry( mapName );
         DatabaseAppendMapEntry( newMapEntry );
      }
   }

   database.currentMap = newMapEntry;
   StrDel( mapName );
   /* PrintMessage( "Map: %s\n", mapName->value ); */

   return TRUE;
}

DatabaseMapEntry *DatabaseCreateMapEntry( const Str *mapName ) {
   DatabaseMapEntry *mapEntry = 
      ( DatabaseMapEntry * ) malloc( sizeof( DatabaseMapEntry ) );

   mapEntry->name = StrCopy( mapName );
   mapEntry->nextEntry = NULL;
   mapEntry->totalRecords = 0;
   mapEntry->firstRecord = NULL;

   return mapEntry;
}

void DatabaseAppendMapEntry( DatabaseMapEntry *entry ) {
   entry->nextEntry = database.firstMap;
   database.firstMap = entry;
   database.totalMaps += 1;
}

const Str *DatabaseGetCurrentMap( void ) {
   return database.currentMap->name;
}

void DatabaseStore( const Str *name, const Str *value ) {
   DatabaseRecord *record;

   if ( name == NULL || value == NULL ) {
      return;
   }

   /* PrintMessage( "Storing: %s = %s\n", name->value, value->value ); */

   /* Search for the record to update. */
   record = database.currentMap->firstRecord;
   while ( record != NULL && ! StrIsEqual( record->key, name ) ) {
      record = record->nextRecord;
   }

   /* Update the existing record. */
   if ( record != NULL ) {
      StrDel( record->value );
      record->value = StrCopy( value );
   }
   /* Otherwise, create a new record for the map if one wasn't found with
      the given key or there are no records for the map at all. */
   else {
      record = ( DatabaseRecord * ) malloc( sizeof( DatabaseRecord ) );
      if ( record == NULL ) {
         return;
      }

      record->key = StrCopy( name );
      record->value = StrCopy( value );

      /* If we failed to append the record, get rid of it. */
      if ( ! DatabaseAppendRecord( record ) ) {
         StrDel( record->key );
         StrDel( record->value );
         free( ( void * ) record );
      }
   }

   /* Indicate an update was made to the database. */
   database.updatesSinceLastSave += 1;
}

Bool DatabaseAppendRecord( DatabaseRecord *record ) {
   /* Only append the record if we haven't gone over the limit of
      maximum records to store. */
   if ( database.totalRecords < DATABASE_MAX_ENTRIES ) {
      DatabaseMapEntry *mapEntry = database.currentMap;

      record->nextRecord = mapEntry->firstRecord;
      mapEntry->firstRecord = record;
      mapEntry->totalRecords += 1;

      database.totalRecords += 1;

      return TRUE;
   }
   else {
      PrintWarning( "Record limit of %d has been reached. Cannot add "
         "anymore records\n", DATABASE_MAX_ENTRIES );
      return FALSE;
   }
}

const Str *DatabaseRetrieve( const Str *name ) {
   DatabaseRecord *record = database.currentMap->firstRecord;
   while ( record != NULL ) {
      if ( record->key->length == name->length &&
         StrIsEqual( record->key, name ) ) {
         break;
      }

      record = record->nextRecord;
   }

   if ( record != NULL ) {
      return record->value;
   }
   else {
      return NULL;
   }
}

Bool DatabaseSave( const char *databaseOutPath ) {
   Bool isSaved = LukdExportDatabase( &database, databaseOutPath );

   if ( isSaved ) {
      database.updatesSinceLastSave = 0;
   }

   return isSaved;
}

void DatabaseShutdown( void ) {
   DatabaseMapEntry *entry;
   DatabaseMapEntry *nextEntry;

   /* Destroy all map entries. */
   entry = database.firstMap;
   while ( entry != NULL ) {
      /* Hold on to the next entry so we can delete the current one. */
      nextEntry = entry->nextEntry;
      DatabaseDestroyMapEntry( entry );
      entry = nextEntry;
   }

   database.updatesSinceLastSave = 0;
   database.isOperational = FALSE;
}

void DatabaseDestroyMapEntry( DatabaseMapEntry *entry ) {
   DatabaseRecord *record = entry->firstRecord;
   DatabaseRecord *nextRecord;

   /* Destroy the records from first. */
   while ( record != NULL ) {
      nextRecord = record->nextRecord;

      StrDel( record->key );
      StrDel( record->value );
      database.totalRecords -= 1;
      free( ( void * ) record );

      record = nextRecord;
   }

   StrDel( entry->name );
   free( ( void * ) entry );

   database.totalMaps -= 1;
}

int DatabaseCalculateRecordsTotalSize( void ) {
   int size = 0;

   DatabaseMapEntry *entry;
   DatabaseRecord *record;

   entry = database.firstMap;
   while ( entry != NULL ) {
      record = entry->firstRecord;

      while ( record != NULL ) {
         size += record->key->length + record->value->length;
         record = record->nextRecord;
      }

      entry = entry->nextEntry;
   }

   return size;
}

Bool DatabaseDelete( const Str *mapName ) {
   DatabaseMapEntry *prevEntry = database.firstMap;
   DatabaseMapEntry *currEntry = database.firstMap;

   while ( currEntry != NULL && ! StrIsEqual( currEntry->name, mapName ) ) {
      prevEntry = currEntry;
      currEntry = currEntry->nextEntry;
   }

   if ( currEntry != NULL ) {
		/* If found entry isn't first entry, link next entry to previous one. */
      if ( currEntry != database.firstMap ) {	
         prevEntry->nextEntry = currEntry->nextEntry;
	   }
	   /* Otherwise, make next entry the first entry. */
	   else {
		   database.firstMap = currEntry->nextEntry;
		}
	   
      DatabaseDestroyMapEntry( currEntry );
      database.updatesSinceLastSave += 1;

      return TRUE;
   }
   else {
      return FALSE;
   }
}

void DatabasePrint( const Str *selectedMap ) {
   DatabaseMapEntry *map;

   PrintMessage( "----- Database -----\n" );
   PrintMessage( "Map entries: %d\n", database.totalMaps );
   PrintMessage( "Records: %d\n", database.totalRecords );
   PrintMessage( "\n" );

   map = database.firstMap;

   /* Print records for single map. */
   if ( selectedMap != NULL ) {
      while ( map != NULL && ! StrIsEqual( selectedMap, map->name ) ) {
         map = map->nextEntry;
      }

      if ( map != NULL ) {
         DatabasePrintMapEntry( map );
      }
      else {
         PrintMessage( "No such map in database: %s\n", 
            selectedMap->value );
      }
   }
   /* Print records for all maps. */
   else {
      while ( map != NULL ) {
         DatabasePrintMapEntry( map );
         map = map->nextEntry;
      }
   }
}

void DatabasePrintMapEntry( const DatabaseMapEntry *entry ) {
   DatabaseRecord *record;

   PrintMessage( "\tName: %s\n", entry->name->value );
   PrintMessage( "\tTotal records: %d\n", ( int ) entry->totalRecords );
   PrintMessage( "\n" );

   /* Print records of the map entry: */
   record = entry->firstRecord;

   while ( record != NULL ) {
      DatabasePrintRecord( record );
      record = record->nextRecord;
   }
}

void DatabasePrintRecord( const DatabaseRecord *record ) {
   PrintMessage( "\t\tKey: %s\n", record->key->value );
   PrintMessage( "\t\tValue: %s\n", record->value->value );
   PrintMessage( "\n" );
}
