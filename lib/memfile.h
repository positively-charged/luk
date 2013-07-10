/*

   This library helps us with manipulating files in memory.
   -- Positron

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

#ifndef MEMFILE_H
#define MEMFILE_H

#include <stdlib.h>

#include "gentype.h"

#define MF_START_MEMORY 1024

/* Error codes for errors that might be encountered when manipulating
   memory files. */
#define MF_ERR_OUT_OF_MEMORY -1
#define MF_ERR_BAD_PATH -2
#define MF_ERR_FILE_READ -3
#define MF_ERR_FILE_WRITE -4
#define MF_ERR_INVALID_POSITION -5

/* When loading a file to append to a memory file, we will read the file
   data into a temporary buffer before adding in to the memory file. */
#define MF_TEMP_BUFFER_SIZE MF_START_MEMORY

typedef struct {
   Byte *data;
   /* Size of allocated memory in bytes. */
   size_t memoryAllocated;
   /* Size of all the data currently in the memory file. */
   size_t size;
   /* Variable to point to a position in the data. */
   size_t pos;
} MemFile;

void MemFileInit( MemFile *memFile );
int MemFileAdd( MemFile *memFile, const void *data, size_t numberOfBytes );
int MemFileAddFile( MemFile *memFile, const char *filePath );
int MemFileAddMemFile( MemFile *memFile, const MemFile *otherMemFile );
int MemFileSave( const MemFile *memFile, const char *outPath );
size_t MemFileRead( MemFile *memFile, void *buffer, size_t numberOfBytes );
int MemFileSetPosition( MemFile *memFile, size_t newPosition );
size_t MemFileGetPosition( const MemFile *memFile );
size_t MemFileGetSize( const MemFile *memFile );
void MemFileRewind( MemFile *memFile );
const char *MemFileGetErrorCodeMessage( int errorCode );
void MemFilePrint( const MemFile *memFile, Bool printData );
void MemFileClose( MemFile *memFile );

#endif
