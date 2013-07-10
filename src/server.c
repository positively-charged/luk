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
#include <stdlib.h>

#include "huffman.h"
#include "md5.h"

#include "server.h"
#include "print.h"
#include "platform.h"

/* Private prototypes: */
static Str *ServerGeneratePasswordHash( const Str *salt, const Str *password );
static Bool ServerWaitForReply( int seconds );

static RconServer server;

Bool ServerInit( const Str *ip, const Str *port ) {
   int portNumber;

   /* Prepare the Huffman encoding subsystem. */
   HUFFMAN_Construct();

   /* Initialize the Socket subsystem. */
   SocketInit();
   atexit( SocketShutdown );

   server.socket = SocketCreate( AF_INET, SOCK_DGRAM, 0 );
   if ( server.socket == SOCKET_FAIL ) {
      PrintError( "Socket creation failure\n" );
      return FALSE;
   }

   server.isLoggedIn = FALSE;

   /* Fill in the socket address structure: */
   memset( &server.address, 0, sizeof( server.address ) );
   server.address.sin_family = AF_INET;
   portNumber = atoi( port->value );
   if ( portNumber > 0 ) {
      server.address.sin_port = htons( portNumber );
   }
   /* Trigger an error if the port number could not be converted
      to a proper integer value. */
   else {
      SocketDestroy( &server.socket );
      PrintError( "Invalid port number given for RCON server: %s\n",
         port->value );
      return FALSE;
   }

   if ( SocketStoreIp( &server.address, ip->value ) == 0 ) {
      SocketDestroy( &server.socket );
      PrintError( "Invalid IP address given for RCON server: %s\n", 
         ip->value );
      return FALSE;
   }

   server.port = StrCopy( port );
   server.ip = StrCopy( ip );

   return TRUE;
}

Str *ServerGeneratePasswordHash( const Str *salt, const Str *password ) {
   md5_byte_t digest[ 16 ];
   int digit;

   /* Combine the salt with the password to form one unit that we
      will encrypt. */
   Str *saltPassword = StrConcat( salt, password );
   Str *hash = StrNewEmpty( MD5_HASH_LENGTH );

   md5_state_t state;
   md5_init( &state );

   md5_append( &state, ( md5_byte_t * ) saltPassword->value, 
      ( int ) saltPassword->length );

   md5_finish( &state, digest );

   for ( digit = 0; digit < 16; digit += 1 ) {
      sprintf( hash->value + digit * 2, "%02x", digest[ digit ] );
   }

   StrDel( saltPassword );

   return hash;
}

int ServerLogin( const Str *password, RconResponse *extResponse, 
   int timeout ) {
   RconResponse response;
   Str *passwordHash;
   Str *salt;

   /* We need to make sure that the client isn't already logged in
      before proceeding to login. */
   if ( server.isLoggedIn ) {
      return SV_ERR_ALREADY_LOGGED_IN;
   }

   /* Connection start response: */
   response.header = CLRC_BEGINCONNECTION;
   response.body[ 0 ] = RCON_VERSION_SUPPORTED;
   response.bodyLength = 1;

   ServerSend( &response );
   if ( ! ServerReceive( &response, timeout ) ) {
      return SV_ERR_TIMEOUT;
   }

   /* Confirm that we can proceed to login in to the server. */
   switch ( response.header ) {
      case SVRC_OLDPROTOCOL:
         PrintError( "The server RCON protocol version is newer\n" );
         return SV_ERR_OLD_PROTOCOL;

      case SVRC_BANNED:
         PrintError( "Your IP address has been banned from the server\n" );
         return SV_ERR_BANNED;
   }
   
   /* Password hash response: */
   salt = StrNew( ( const char * ) response.body );
   passwordHash = ServerGeneratePasswordHash( salt, password );

   response.header = CLRC_PASSWORD;
   /* Notice that we also copy the NULL character into the response body. */
   memcpy( response.body, passwordHash->value, passwordHash->length + 1 );
   response.bodyLength = passwordHash->length + 1;

   StrDel( salt );
   StrDel( passwordHash );

   ServerSend( &response );
   if ( ! ServerReceive( extResponse, timeout ) ) {
      return SV_ERR_TIMEOUT;
   }

   if ( extResponse->header == SVRC_LOGGEDIN ) {
      server.isLoggedIn = TRUE;
      return SV_ERR_NONE;
   }
   else if ( extResponse->header == SVRC_INVALIDPASSWORD ) {
      PrintError( "Incorrect password for RCON server given\n" );
      return SV_ERR_INVALID_PASSWORD;
   }
   /* Anything unknown we simply abort. */
   else {
      return SV_ERR_UNKNOWN;
   }
}

Bool ServerWaitForReply( int seconds ) {
   fd_set serverFdSet;
   struct timeval timeout;

   timeout.tv_sec = seconds;
   timeout.tv_usec = 0;

   FD_ZERO( &serverFdSet );
   FD_SET( server.socket, &serverFdSet );

   return select( server.socket + 1, &serverFdSet, NULL, NULL, &timeout ) > 0;
}

void ServerSend( RconResponse *response ) {
   unsigned char encoded[ MAX_RESPONSE_LENGTH ];
   int encodedLen = MAX_RESPONSE_LENGTH;

   HUFFMAN_Encode( ( unsigned char * ) response, encoded, 
      response->bodyLength + 1, &encodedLen );

   sendto( server.socket, ( const char * ) encoded, encodedLen, 0,
      ( struct sockaddr * ) &server.address, sizeof( server.address ) );
}

void ServerSendCommand( const Str *consoleCommand ) {
   RconResponse response;

   response.header = CLRC_COMMAND;
   memcpy( response.body, consoleCommand->value, consoleCommand->length + 1 );
   response.bodyLength = consoleCommand->length + 1;

   ServerSend( &response );
   PrintMessage( "   -> %s\n", consoleCommand->value );
}

void ServerSendCommandC( const char *consoleCommand ) {
   const Str *command = StrNew( consoleCommand );
   ServerSendCommand( command );
   StrDel( command );
}

Bool ServerReceive( RconResponse *response, int timeout ) {
   if ( ServerWaitForReply( timeout ) ) {
      unsigned char encoded[ MAX_RESPONSE_LENGTH ];
      struct sockaddr_in remoteAddr;
      socklen_t remoteAddrLen = sizeof( remoteAddr );
      int responseLen = 0;

      int encodedLen = recvfrom( server.socket, ( char* ) encoded, sizeof( encoded ),
         0, ( struct sockaddr * ) &remoteAddr, &remoteAddrLen );
      if ( encodedLen == -1 ) {
		   return FALSE;
	   }

      /* Bail out if the remote address isn't that of the server. */
      if ( remoteAddrLen != sizeof( server.address ) ||
         memcmp( &remoteAddr, &server.address, remoteAddrLen ) ) {
         PrintNotice( "Ignoring query from unknown host: %s:%d\n",
            inet_ntoa( remoteAddr.sin_addr ), ntohs( remoteAddr.sin_port ) );
         return FALSE;
      }

      /* Remove a single byte for the precaution below. */
      responseLen = MAX_RESPONSE_LENGTH - 1;
      HUFFMAN_Decode( encoded, ( unsigned char * ) response, encodedLen,
         &responseLen );
      if ( ! responseLen ) {
         return FALSE;
      }

      /* We don't include the header as part of the body, so remove it
         from the body length. */
      response->bodyLength = responseLen - 1;
      /* As an extra precaution, we append a NULL character to the end of
         the response in case the data became malformed during transmission. */
      response->body[ responseLen ] = 0;

      return TRUE;
   }
   else {
      return FALSE;
   }
}

void ServerDisconnect( void ) {
   RconResponse response;

   response.header = CLRC_DISCONNECT;
   response.bodyLength = 0;

   ServerSend( &response );
}

void ServerShutdown( void ) {
   if ( server.isLoggedIn ) {
      ServerDisconnect();
      server.isLoggedIn = FALSE;
   }

   StrDel( server.ip );
   StrDel( server.port );

   server.ip = NULL;
   server.port = NULL;

   SocketDestroy( &server.socket );
}
