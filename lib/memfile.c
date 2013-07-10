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
#include <string.h>

#include "memfile.h"

void MemFileInit( MemFile *memFile ) {
   memFile->data = NULL;
   memFile->memoryAllocated = 0;
   memFile->size = 0;
   memFile->pos = 0;
}

int MemFileAdd( MemFile *memFile, const void *data, size_t numberOfBytes ) {
   /* If adding the new data will go over the currently allocated memory, we 
      need to reallocate before proceeding. */
   if ( memFile->pos + numberOfBytes > memFile->memoryAllocated ) {
      const size_t newAmount = ( memFile->pos + numberOfBytes ) * 2;
      Byte *newBlock = ( Byte * ) realloc( memFile->data, newAmount );
      if ( newBlock != NULL ) {
         memFile->data = newBlock;
         memFile->memoryAllocated = newAmount;
      }
      else {
         return MF_ERR_OUT_OF_MEMORY;
      }
   }

   memcpy( memFile->data + memFile->pos, data, numberOfBytes );
   memFile->pos += numberOfBytes;
   /* Update the file size if the position went over the previously
      set size. */
   if ( memFile->pos > memFile->size ) {
      memFile->size = memFile->pos;
   }

   return numberOfBytes;
}

int MemFileAddFile( MemFile *memFile, const char *filePath ) {
   Byte segment[ MF_TEMP_BUFFER_SIZE ];
   size_t segmentLength;
   size_t fileBytesAdded = 0;

   FILE *fileHandle = fopen( filePath, "rb" );
   if ( fileHandle == NULL ) {
      return MF_ERR_BAD_PATH;
   }

   while ( ! feof( fileHandle ) ) {
      segmentLength = fread( segment, sizeof( Byte ), 
         MF_TEMP_BUFFER_SIZE, fileHandle );
      /* Break out of the loop if we encounter read errors. */
      if ( ferror( fileHandle ) ) {
         fileBytesAdded = MF_ERR_FILE_READ; 
         break;
      }

      if ( segmentLength > 0 ) {
         int bytesAdded = MemFileAdd( memFile, segment, segmentLength );
         /* Break out of the loop if an error was encountered while trying
            to add the current file data segment. */
         if ( bytesAdded >= 0 ) {
            fileBytesAdded += bytesAdded;
         }
         else {
            fileBytesAdded = bytesAdded;
            break;
         }
      }
   }

   fclose( fileHandle );
   return fileBytesAdded;
}

int MemFileAddMemFile( MemFile *memFile, const MemFile *otherMemFile ) {
   return MemFileAdd( memFile, otherMemFile->data, otherMemFile->size );
}

int MemFileSave( const MemFile *memFile, const char *outPath ) {
   size_t bytesWritten;

   FILE *outFileHandle = fopen( outPath, "wb" );
   if ( outFileHandle == NULL ) {
      return MF_ERR_BAD_PATH;
   }

   bytesWritten = fwrite( memFile->data, sizeof( Byte ), 
      memFile->size, outFileHandle );
   fclose( outFileHandle );

   /* Return the number of bytes written if all is well; trigger an error if
      not all of the elements were written to file. */
   if ( bytesWritten == memFile->size ) {
      return bytesWritten;
   }
   else {
      return MF_ERR_FILE_WRITE;
   }
}

size_t MemFileRead( MemFile *memFile, void *buffer, size_t numberOfBytes ) {
   /* If the number of bytes is larger than the end position, 
      read the number of bytes up to the end position. */
   size_t bytesToRead = numberOfBytes;
   if ( memFile->pos + numberOfBytes > memFile->size ) {
      bytesToRead = memFile->size - memFile->pos;
   }

   if ( bytesToRead > 0 ) {
      memcpy( buffer, memFile->data + memFile->pos, bytesToRead );
      memFile->pos += bytesToRead;
   }

   return bytesToRead;
}

int MemFileSetPosition( MemFile *memFile, size_t newPosition ) {
   /* Only change to the new position if it's within the current data size,
      including the data size. */
   if ( newPosition <= memFile->size ) {
      memFile->pos = newPosition;
      return newPosition;
   }
   else {
      return MF_ERR_INVALID_POSITION;
   }
}

size_t MemFileGetPosition( const MemFile *memFile ) {
   return memFile->pos;
}

size_t MemFileGetSize( const MemFile *memFile ) {
   return memFile->size;
}

void MemFileRewind( MemFile *memFile ) {
   MemFileSetPosition( memFile, 0 );
}

const char *MemFileGetErrorCodeMessage( int errorCode ) {
   switch ( errorCode ) {
      case MF_ERR_BAD_PATH: return "Invalid path given";
      case MF_ERR_FILE_READ: return "File read error encountered";
      case MF_ERR_FILE_WRITE: return "File write error encountered";
      case MF_ERR_INVALID_POSITION: return "Invalid position";
      case MF_ERR_OUT_OF_MEMORY: return "Memory allocation failure";
      default: return NULL;
   }
}

void MemFilePrint( const MemFile *memFile, Bool printData ) {
   printf( "Size: %d\n", ( int ) memFile->size );
   printf( "Memory allocated: %d\n", ( int ) memFile->memoryAllocated );
   printf( "File position: %d\n", ( int ) memFile->pos );
   if ( printData ) {
      printf( "Data: \n" );
      printf( "%s\n", memFile->data );
   }
}

void MemFileClose( MemFile *memFile ) {
   if ( memFile->data != NULL ) {
      free( ( void * ) memFile->data );
   }

   memFile->memoryAllocated = 0;
   memFile->size = 0;
   memFile->pos = 0;
}
