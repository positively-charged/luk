/*

   This header contains a configuration file template in a macro that we
   will use to generate a fresh copy of a configuration file.

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
   
#define LUK_COFIG_FILE_TEMPLATE \
   "# The IP address of the RCON server. You can use the special value\n" \
   "# \"localhost\" to refer to the current machine as the host.\n" \
   "server_address = \"localhost\"\n" \
   "# The port number of the server.\n" \
   "server_port = \"10666\"\n" \
   "# Enter the RCON password that the server uses for logging in.\n" \
   "server_password = \"\"\n" \
   "\n" \
   "# Enter a file path to where you would like to have the database file\n" \
   "# stored at. The database file stores data that the RCON server passes " \
   "to it.\n" \
   "database_path = \"./database.lukd\""
