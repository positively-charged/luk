/*

   Socket handling file used to hide the system-dependent code.

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

#ifndef SOCKET_H
#define SOCKET_H

/* Windows: */
#if defined _WIN32 || defined _WIN64
   #include <winsock2.h>

   #define SocketClose closesocket
   #define SOCKET_FAIL INVALID_SOCKET

   typedef SOCKET Socket;

   #define socklen_t int

/* Linux: */
#else
   #include <netinet/in.h>
   #include <unistd.h>
   #include <sys/socket.h>
   #include <sys/select.h>
   #include <sys/time.h>
   #include <arpa/inet.h>

   #define SocketClose close
   #define SOCKET_FAIL -1

   typedef int Socket;
#endif

/* Prototypes: */
void SocketInit( void );
Socket SocketCreate( int family, int type, int protocol );
int SocketStoreIp( struct sockaddr_in *address, const char *ip );
void SocketDestroy( const Socket *socket );
void SocketShutdown( void );

#endif
