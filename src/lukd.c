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
#include <time.h>

#include "memfile.h"

#include "lukd.h"
#include "database.h"
#include "print.h"

/* Import functions: */
static Bool LukdImport( MemFile *dataFile );
static Bool LukdImportMapEntries( MemFile *dataFile, 
   const LukdMainTable *table, int *totalRecords );
static Bool LukdImportRecords( MemFile *dataFile, const LukdMapEntry *entry,
   int *totalRecords );
/* Validation functions: */
static Bool LukdIsValidMainTableOffset( LukdMainTableOffset offset,
   const size_t fileSize );
static Bool LukdIsValidMainTable( const LukdMainTable *table,
   const size_t fileSize );
static Bool LukdIsValidMapEntry( const LukdMapEntry *entry, 
   const size_t fileSize );
static Bool LukdIsValidRecordHeader( const LukdRecordHeader *header, 
   const MemFile *file );
static void LukdBackupFile( MemFile *dataFile, const char *dataFilePath );
static void LukdPrintFileInfo( const LukdMainTable *table, int totalRecords );
/* Export functions */
static int LukdExportEntries( MemFile *outFile, const Database *database,
   size_t *firstMapEntry );
static int LukdExportRecords( MemFile *outFile, DatabaseMapEntry *dbEntry );
static void LukdExportMainTable( MemFile *outFile, 
   unsigned int totalMapEntries, unsigned int firstMapEntry );
/* Debug functions: */
static void LukdPrintMainTable( const LukdMainTable *table );
static void LukdPrintEntry( const LukdMapEntry *entry );

Bool LukdImportDatabase( const char *dataFilePath ) {
   Bool isImported = FALSE;

   int bytesAdded;
   MemFile dataFile;
   MemFileInit( &dataFile );

   PrintMessage( "Importing database file at path: %s\n", dataFilePath );
   bytesAdded = MemFileAddFile( &dataFile, dataFilePath );
   if ( bytesAdded > 0 ) {
      /* Then proceed to import the records. */
      isImported = LukdImport( &dataFile );
      /* If we imported the database file successfully, make a backup. */
      if ( isImported ) {
         LukdBackupFile( &dataFile, dataFilePath );
      }
   }
   /* Bail out successfully if the database file is empty, meaning it
      must have been created by the user manually. */
   else if ( bytesAdded == 0 ) {
      PrintNotice( "Database file is empty\n" );
      isImported = TRUE;
   }
   /* Trigger an error if the file was not read successfully. */
   else {
      const int errorCode = bytesAdded;
      PrintWarning( "Failed to import database file at path: %s\n",
         dataFilePath );
      /* Print the reason for the error: */
      PrintMessage( "Reason for failure: %s\n",
         MemFileGetErrorCodeMessage( errorCode ) );
   }

   MemFileClose( &dataFile );
   return isImported;
}

Bool LukdImport( MemFile *dataFile ) {
   Bool isImported = FALSE;

   const size_t dataFileSize = MemFileGetSize( dataFile );
   size_t bytesRead;

   LukdMainTableOffset mainTableOffset;
   LukdMainTable mainTable;
   const size_t mainTableSize = sizeof( mainTable );
   int totalRecords;

   /* Collect the main table offset and do some sanity checks on it. */
   MemFileRewind( dataFile );
   MemFileRead( dataFile, &mainTableOffset, sizeof( mainTableOffset ) );
   if ( ! LukdIsValidMainTableOffset( mainTableOffset, dataFileSize ) ) {
      PrintWarning( "Bad main table offset in file: %lu\n", mainTableOffset );
      return FALSE;
   }

   /* Collect the main table. */
   MemFileSetPosition( dataFile, mainTableOffset );
   bytesRead = MemFileRead( dataFile, &mainTable, mainTableSize );
   /* Bail out if the main table read is not of required size or is
      an invalid one. */
   if ( bytesRead != mainTableSize || 
      ! LukdIsValidMainTable( &mainTable, dataFileSize ) ) {
      PrintWarning( "Corrupt main table in database file detected\n" );
      return FALSE;
   }

   /* Import the map entries and their records. */
   if ( LukdImportMapEntries( dataFile, &mainTable, &totalRecords ) ) {
      /* If all is well, print the information about the file. */
      LukdPrintFileInfo( &mainTable, totalRecords );
      isImported = TRUE;
   }

   return isImported;
}

Bool LukdImportMapEntries( MemFile *dataFile, const LukdMainTable *table,
   int *totalRecords ) {
   const size_t dataFileSize = MemFileGetSize( dataFile );

   size_t nextEntryPosition;
   LukdMapEntry entry;
   unsigned int entryNum;

   /* Begin by moving to the position of the first map entry. */
   MemFileSetPosition( dataFile, table->firstMapEntry );

   *totalRecords = 0;
   for ( entryNum = 0; entryNum < table->totalMapEntries; entryNum += 1 ) {
      MemFileRead( dataFile, &entry, sizeof( LukdMapEntry ) );
      /* Do some sanity checks on the map entry. */
      if ( LukdIsValidMapEntry( &entry, dataFileSize ) ) {
         /* Change to the appropriate map in the database: */
         Str *mapName = StrNewSub( entry.name, LUKD_MAX_MAP_LENGTH );
         DatabaseChangeMap( mapName );
         StrDel( mapName );

         /* Import the records, but remember the file position of the
            next map entry. */
         nextEntryPosition = MemFileGetPosition( dataFile );
         if ( ! LukdImportRecords( dataFile, &entry, totalRecords ) ) {
            return FALSE;
         }
         /* Restore the position of the next map entry. */
         MemFileSetPosition( dataFile, nextEntryPosition );
      }
      else {
         PrintWarning( 
            "Corrupt map entry encountered in database file\n" );
         return FALSE;
      }
   }

   return TRUE;
}

Bool LukdImportRecords( MemFile *dataFile, const LukdMapEntry *entry,
   int *totalRecords ) {
   LukdRecordHeader recordHeader;
   unsigned int recordNum;

   /* Move to the first record in the record series of the map entry. */
   MemFileSetPosition( dataFile, entry->firstRecord );

   for ( recordNum = 0; recordNum < entry->totalRecords; recordNum += 1 ) {
      MemFileRead( dataFile, &recordHeader, sizeof( recordHeader ) );
      if ( LukdIsValidRecordHeader( &recordHeader, dataFile ) ) {
         Str *key = StrNewEmpty( recordHeader.keySize );
         Str *value = StrNewEmpty( recordHeader.valueSize );
         if ( key == NULL || value == NULL ) {
            PrintWarning( 
               "Failed to allocate enough memory for a record\n" );
            StrDel( key );
            StrDel( value );
            return FALSE;
         }

         MemFileRead( dataFile, key->value, key->length );
         MemFileRead( dataFile, value->value, value->length );

         /* Load the record into the database: */
         DatabaseStore( key, value );

         StrDel( key );
         StrDel( value );

         *totalRecords += 1;
      }
      /* Finish processing the records if we find an invalid
         record. */
      else {
         PrintWarning( "Malformed record found in database file\n" );
         return FALSE;
      }
   }

   return TRUE;
}

/* Validation functions */

Bool LukdIsValidMainTableOffset( LukdMainTableOffset offset,
   const size_t fileSize ) {
   LukdMainTableOffset tableMaxOffset = fileSize - sizeof( LukdMainTable );
   return ( offset <= tableMaxOffset );
}

Bool LukdIsValidMainTable( const LukdMainTable *table,
   const size_t fileSize ) {
   /* We only validate the other fields if there are actually map entries
      present in the file. */
   if ( table->totalMapEntries > 0 ) {
      const size_t mapEntrySize = sizeof( LukdMapEntry );

      /* Make sure that the first map entry is not off limits. */
      unsigned int entryStartLowerLimit = sizeof( LukdMainTable );
      unsigned entryStartUpperLimit = fileSize - mapEntrySize;

      if ( table->firstMapEntry < entryStartLowerLimit ||
         table->firstMapEntry > entryStartUpperLimit ) {
         PrintWarning( "First map entry is NOT within valid limits\n" );
         return FALSE;
      }

      /* Make sure the total map entries given is enough to fit in the
         target file. */
      if ( table->totalMapEntries * mapEntrySize >
         fileSize - table->firstMapEntry ) {
         PrintWarning( "Total size of entries is too big for given file\n" );
         return FALSE;
      }
   }

   return TRUE;
}

Bool LukdIsValidMapEntry( const LukdMapEntry *entry, const size_t fileSize ) {
   if ( entry->totalRecords > 0 ) {
      unsigned int RecordStartLowerLimit = sizeof( LukdMainTableOffset );
      unsigned int RecordStartUpperLimit = fileSize - 
         sizeof( LukdRecordHeader );

      /* Make sure the start of the first record is not off limits. */
      if ( entry->firstRecord < RecordStartLowerLimit ||
         entry->firstRecord > RecordStartUpperLimit ) {
         return FALSE;
      }

      /* Make sure the total records is not beyond the file size. 
         FIXME: This is bad and needs improving. */
      if ( entry->totalRecords >= fileSize ) {
         return FALSE;
      }
   }

   return TRUE;
}

Bool LukdIsValidRecordHeader( const LukdRecordHeader *header, 
   const MemFile *file ) {
   /* Find the maximum size of a record based on the current position
      of the file and the file information. Then compare this value
      with the current record information to see if it fits within
      the current limit. */
   const size_t currentMaxRecordBodySize = MemFileGetSize( file ) - 
      sizeof( LukdMainTable ) - MemFileGetPosition( file );
   const size_t currentRecordBodySize = header->keySize + header->valueSize;
   return ( currentRecordBodySize <= currentMaxRecordBodySize );
}

void LukdPrintFileInfo( const LukdMainTable *table, int totalRecords ) {
   char publishDate[ LUKD_PUBLISH_DATE_MAX_LENGTH ];

   time_t time = ( time_t ) table->publishDate;
   struct tm *localTime = localtime( &time );

   strftime( publishDate, LUKD_PUBLISH_DATE_MAX_LENGTH, 
      LUKD_PUBLISH_DATE_FORMAT, localTime );

   /* Print all the important information about the file to the user. */
   PrintMessage( "Database file: \n" );
   PrintMessage( "   - Published on: %s\n", publishDate );
   PrintMessage( "   - Total map entries: %lu\n", table->totalMapEntries ); 
   PrintMessage( "   - Total records: %lu\n", totalRecords ); 
}

void LukdBackupFile( MemFile *dataFile, const char *dataFilePath ) {
   int bytesSaved;

   const Str *backupFilePath = 
      StrNewEmpty( strlen( dataFilePath ) + strlen( LUKD_BACKUP_EXT ) );
   if ( backupFilePath == NULL ) {
      return;
   }
   sprintf( backupFilePath->value, "%s%s", dataFilePath, LUKD_BACKUP_EXT );

   PrintMessage( "Creating backup database file at path: %s\n",
      backupFilePath->value );

   bytesSaved = MemFileSave( dataFile, backupFilePath->value );
   if ( bytesSaved < 0 ) {
      const int errorCode = bytesSaved;
      PrintWarning( "Failed to create a backup of the database file\n" );
      PrintMessage( "Reason for failure: %s\n", 
         MemFileGetErrorCodeMessage( errorCode ) );
   }

   StrDel( backupFilePath );
}

/* Functions to write the database to file. */

Bool LukdExportDatabase( const Database *database, const char *outFilePath ) {
   size_t firstMapEntry;
   int entriesExported;
   int bytesWritten;
   Bool isExported;

   LukdMainTableOffset mainTableOffset = 0;
   const size_t mainTableOffsetSize = sizeof( mainTableOffset );

   MemFile outFile;
   MemFileInit( &outFile );

   PrintMessage( "Saving database to path: %s\n", outFilePath );

   /* Prepare the space for the main table offset. */
   MemFileAdd( &outFile, &mainTableOffset, mainTableOffsetSize );

   /* Export the map entries and their records. */
   entriesExported = LukdExportEntries( &outFile, database, &firstMapEntry );
   /* After we add the map entries and their records into the output file,
      we need to collect the current position of the file because the next
      item to be added will be the main table and we need the offset of 
      the main table in the beginning of the lukd file. */
   mainTableOffset = MemFileGetPosition( &outFile );

   /* Export the main table with the map entries data collected above. */
   LukdExportMainTable( &outFile, entriesExported, firstMapEntry );

   /* Now record the main table offset. */
   MemFileRewind( &outFile );
   MemFileAdd( &outFile, &mainTableOffset, mainTableOffsetSize );

   /* Save the file contents into a permanent output file: */
   bytesWritten = MemFileSave( &outFile, outFilePath );
   if ( bytesWritten >= 0 ) {
      isExported = TRUE;
   }
   else {
      const int errorCode = bytesWritten;
      PrintWarning( "Could not write to file at path: %s\n", outFilePath );
      PrintMessage( "Reason for failure: %s\n", 
         MemFileGetErrorCodeMessage( errorCode ) );
      isExported = FALSE;
   }

   MemFileClose( &outFile );
   return isExported;
}

int LukdExportEntries( MemFile *outFile, const Database *database,
   size_t *firstMapEntry ) {   
   LukdMapEntry lukdEntry;
   DatabaseMapEntry *dbEntry = database->firstMap;

   int entriesExported = 0;
   int recordsExported;

   size_t firstRecordPosition;

   /* We will save the map entries into a separate memory file and then
      append its contents into the output file. */
   MemFile entriesFile;
   MemFileInit( &entriesFile );

   while ( dbEntry != NULL ) {
      firstRecordPosition = MemFileGetPosition( outFile );
      recordsExported = LukdExportRecords( outFile, dbEntry );

      /* Only add a map entry if it has any records. No point in storing
         an empty map entry. */
      if ( recordsExported > 0 ) {
         memset( lukdEntry.name, 0, LUKD_MAX_MAP_LENGTH );
         memcpy( lukdEntry.name, dbEntry->name->value, dbEntry->name->length );

         lukdEntry.totalRecords = recordsExported;
         lukdEntry.firstRecord = firstRecordPosition;

         MemFileAdd( &entriesFile, &lukdEntry, sizeof( lukdEntry ) );
         entriesExported += 1;
      }

      dbEntry = dbEntry->nextEntry;
   }

   /* After we add the records, the next item that will come is the map
      entry directory. The main table needs the start of this directory,
      so we save it. */
   *firstMapEntry = MemFileGetPosition( outFile );

   /* Then we add the collected entries to the output file. */
   MemFileAddMemFile( outFile, &entriesFile );
   MemFileClose( &entriesFile );

   return entriesExported;
}

int LukdExportRecords( MemFile *outFile, DatabaseMapEntry *dbEntry ) {
   int recordsExported = 0;

   DatabaseRecord *record = dbEntry->firstRecord;
   while ( record != NULL ) {
      /* Write record header: */
      LukdRecordHeader lukdRecordHeader;

      lukdRecordHeader.keySize = record->key->length;
      lukdRecordHeader.valueSize = record->value->length;

      MemFileAdd( outFile, &lukdRecordHeader, sizeof( lukdRecordHeader ) );

      /* Write record body: */
      MemFileAdd( outFile, record->key->value, record->key->length );
      MemFileAdd( outFile, record->value->value, record->value->length );

      record = record->nextRecord;
      recordsExported += 1;
   }

   return recordsExported;
}

void LukdExportMainTable( MemFile *outFile, unsigned int totalMapEntries, 
   unsigned int firstMapEntry ) {
   LukdMainTable mainTable;

   mainTable.totalMapEntries = totalMapEntries;
   mainTable.firstMapEntry = firstMapEntry;
   mainTable.publishDate = ( unsigned int ) time( NULL );

   MemFileAdd( outFile, &mainTable, sizeof( mainTable ) );
}

/* Debug functions: */

void LukdPrintMainTable( const LukdMainTable *table ) {
   printf( "Header:\n" );
   printf( "First map entry: %d\n", table->firstMapEntry );
   printf( "Total map entries: %d\n", table->totalMapEntries );
   printf( "Publish date: %d\n", table->publishDate );
   printf( "\n" );
}

void LukdPrintEntry( const LukdMapEntry *entry ) {
   printf( "Map entry:\n" );
   printf( "name: %s\n", entry->name );
   printf( "First record: %d\n", entry->firstRecord );
   printf( "Total records: %d\n", entry->totalRecords );
   printf( "\n" );
}
