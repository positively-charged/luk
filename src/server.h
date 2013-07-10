/*

   Server handling function prototypes.

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

#ifndef SERVER_H
#define SERVER_H

#include "socket.h"
#include "gentype.h"
#include "strutil.h"

#define SERVER_REPLY_TIMEOUT 0
#define MAX_RESPONSE_LENGTH 8192
#define RCON_VERSION_SUPPORTED 3
#define MD5_HASH_LENGTH 32
#define KEEP_ALIVE_REBROADCAST_TIME 5 /* In seconds */

/* RCON client headers. */
typedef enum {
   CLRC_BEGINCONNECTION = 52,
   CLRC_PASSWORD,
   CLRC_COMMAND,
   CLRC_PONG,
   CLRC_DISCONNECT
} ClientMessageHeader;

/* RCON server headers. */
typedef enum {
   SVRC_OLDPROTOCOL = 32,
   SVRC_BANNED,
   SVRC_SALT,
   SVRC_LOGGEDIN,
   SVRC_INVALIDPASSWORD,
   SVRC_MESSAGE,
   SVRC_UPDATE
} ServerMessageHeader;

/* Sub commands of the RCON UPDATE command. */
typedef enum {
   SVRCU_PLAYERDATA = 0,
	SVRCU_ADMINCOUNT,
	SVRCU_MAP
} ServerUpdateMessageHeader;

typedef struct {
   Byte header;
   Byte body[ MAX_RESPONSE_LENGTH ];
   unsigned int bodyLength;
} RconResponse;

/* RCON server communication structure. */
typedef struct {
   /* General server information: */
   Str *ip;
   Str *port;
   Bool isLoggedIn;
   /* Socket information: */
   Socket socket;
   struct sockaddr_in address;
} RconServer;

enum {
   SV_ERR_NONE = 0,
   SV_ERR_ALREADY_LOGGED_IN,
   SV_ERR_INVALID_PASSWORD,
   SV_ERR_BANNED,
   SV_ERR_OLD_PROTOCOL,
   SV_ERR_TIMEOUT,
   SV_ERR_UNKNOWN
};

/* Public prototypes: */
Bool ServerInit( const Str *ip, const Str *port );
int ServerLogin( const Str *password, RconResponse *extResponse, 
   int timeout );
void ServerSend( RconResponse *response );
void ServerSendCommand( const Str *consoleCommand );
void ServerSendCommandC( const char *consoleCommand );
Bool ServerReceive( RconResponse *response, int timeout );
void ServerDisconnect( void );
void ServerShutdown( void );

#endif
