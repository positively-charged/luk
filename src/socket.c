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

#include "socket.h"

void SocketInit( void ) {
   #if defined _WIN32 || defined _WIN64

	WORD sockVersion;
	WSADATA wsaData;

	sockVersion = MAKEWORD( 1, 1 );
	WSAStartup( sockVersion, &wsaData );

   #endif
}

Socket SocketCreate( int family, int type, int protocol ) {
   return socket( family, type, protocol );
}

int SocketStoreIp( struct sockaddr_in *address, const char *ip ) {
   int conversionResult;

   #if defined _WIN32 || defined _WIN64

   conversionResult = inet_addr( ip );
   address->sin_addr.s_addr = conversionResult;

   #else

   conversionResult = inet_pton( AF_INET, ip, &address->sin_addr );

   #endif

   return conversionResult;
}
void SocketDestroy( const Socket *socket ) {
   SocketClose( *socket );
}

void SocketShutdown( void ) {
   #if defined _WIN32 || defined _WIN64

      WSACleanup();

   #endif
}
