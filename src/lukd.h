/*

   Functions to parse and store the values found inside the lukd file 
   into the luk database.

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

#ifndef LUKD_H
#define LUKD_H

#include "gentype.h"
#include "strutil.h"

#include "database.h"

#define LUKD_BACKUP_EXT ".backup"
#define LUKD_MAX_MAP_LENGTH 8  /* Like maximum lump name length. */
#define LUKD_PUBLISH_DATE_MAX_LENGTH 64
/* Make sure the final string, once expanded with arguments, doesn't go
   over the above limit. */
#define LUKD_PUBLISH_DATE_FORMAT "%Y-%m-%d %X %Z"

/* Main table offset: */
typedef unsigned int LukdMainTableOffset;

/* Main table: */
typedef struct {
   unsigned int totalMapEntries;
   unsigned int firstMapEntry;
   int publishDate;
} LukdMainTable;

/* Map entry: */
typedef struct {
   char name[ LUKD_MAX_MAP_LENGTH ];
   unsigned int totalRecords;
   unsigned int firstRecord;
} LukdMapEntry;

/* Record header: */
typedef struct {
   unsigned int keySize;
   unsigned int valueSize;
} LukdRecordHeader;

/* Record body: */
typedef struct {
   Byte *key;
   Byte *value;
} LukdRecordBody;

/* Record: */
typedef struct {
   LukdRecordHeader header;
   LukdRecordBody body;
} LukdRecord;

Bool LukdImportDatabase( const char *dataFilePath );
Bool LukdExportDatabase( const Database *database, const char *outFilePath );

#endif
