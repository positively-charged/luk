/* 

   luk entry point.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "gentype.h"
#include "strutil.h"
#include "progargs.h"
#include "conf.h"

#include "query.h"
#include "handler.h"
#include "reply.h"
#include "command.h"
#include "server.h"
#include "database.h"
#include "config.h"
#include "print.h"
#include "configuration_file_template.h"

/* Private prototypes: */
static Bool LukInitConfigSystem( Bool viewParams );
static Bool LukInitDatabase( void );
static Bool LukInitServer( void );
static Bool LukProcessInitialReponse( const RconResponse *response );
static void LukShutdownConfigSystem( void );
static void LukCloseDatabase( void );
static void LukProcessResponse( const RconResponse *response );
static void LukShutdownServer( void );
static void LukProcessMessageResponse( const Str *message );
static void LukChangeMap( const Str *map );
static void LukSaveDatabase( void );
static void LukExit( int signal );
static void LukPrintCurrentMap( void );
static Bool LukIsRunning( void );
static void LukPrintHelpMenu( const char *programPath );
static void LukGenerateNewConf( void );
static void LukViewProgramType( void );
static Bool LukDeleteMapEntry( void );

static Bool lukIsRunning = TRUE;
static LukMode runMode = LUK_MODE_NORMAL;
static Bool saveDatabaseOnStore = FALSE;

int main( int argc, char *argv[] ) {
   RconResponse response;
   RconResponse pongResponse = { CLRC_PONG, { 0 }, 0 };
   Bool isViewConfigParamGiven = FALSE;

   int replyStatus;
   time_t nextPongTime = 0;

   PROGA_Init( argv );
   ( void ) argc;

   /* If the help argument was provided to the program, display
      the help menu and end execution. */
   if ( PROGA_FindArg( "-h" ) ) {
      LukPrintHelpMenu( argv[ 0 ] );
      exit( EXIT_SUCCESS );
   }
   else if ( PROGA_FindArg( "-g" ) ) {
      LukGenerateNewConf();
      exit( EXIT_SUCCESS );
   }
   else if ( PROGA_FindArg( "-s" ) ) {
      runMode = LUK_MODE_SKIP;
   }

   if ( PROGA_FindArg( "-p" ) ) {
      isViewConfigParamGiven = TRUE;
   }

   signal( SIGINT, LukExit );

   /* Prepare the configuration parameters. */
   if ( LukInitConfigSystem( isViewConfigParamGiven ) ) {
      atexit( LukShutdownConfigSystem );
      if ( isViewConfigParamGiven ) {
         exit( EXIT_SUCCESS );
      }
   }
   else {
      exit( EXIT_FAILURE );
   }

   /* Load data from database. */
   if ( LukInitDatabase() ) {
      atexit( LukCloseDatabase );

      if ( PROGA_FindArg( "-d" ) ) {
         exit( ! LukDeleteMapEntry() );
      }
   }
   else {
      exit( EXIT_FAILURE );
   }

   /* Establish a connection with the RCON server. */
   if ( ! LukInitServer() ) {
      exit( EXIT_FAILURE );
   }

   atexit( HandlerExit );

   /* Begin reading input from the server. */
   PrintMessage( "=====================================================\n" );
   LukPrintCurrentMap();

   /* Turn on the luk system on the server. */
   ServerSendCommandC( "set luk_system 1" );
   while ( LukIsRunning() ) {
      /* Send a stay alive message to the server to stay connected. */
      if ( time( 0 ) >= nextPongTime ) {
         nextPongTime = time( 0 ) + 5;
         ServerSend( &pongResponse );
      }

      replyStatus = ServerReceive( &response, LUK_REPLAY_WAIT_TIME );
      if ( replyStatus > 0 ) {
         LukProcessResponse( &response );
      }
   }

   PrintMessage( "=====================================================\n" );
   PrintMessage( "Shutting down\n" );
   
   ServerSendCommandC( "set luk_system 0" );
   exit( EXIT_SUCCESS );
}

Bool LukInitConfigSystem( Bool viewParams ) {
   Conf configFile;
   Bool isOpen;
   Bool isPopulated = FALSE;

   /* Get the path to the configuration file.*/
   const char *configPath = NULL;

   if ( PROGA_FindArg( "-c" ) ) {
      configPath = PROGA_NextArg();
   }

   /* If one is NOT specified, look for a default configuration file in
      the current directory instead. */
   if ( configPath == NULL ) {
      configPath = LUK_DEFAULT_CONFIG_FILE_PATH;
   }

   /* We only show other messages if we are not displaying the loaded
      configuration parameters. */
   if ( ! viewParams ) {
      PrintMessage( 
         "Reading configuration file at path: %s\n", configPath );
   }

   isOpen = ConfOpen( &configFile, configPath );
   if ( isOpen ) {
      ConfSetReadErrorViewer( &configFile, PrintConfError );
   }
   else {
      PrintError( "Failed to the read configuration file at path: %s\n",
         configPath );
      return FALSE;
   }

   if ( ConfRead( &configFile ) ) {
      isPopulated = ConfigPopulate( &configFile );
   }

   ConfClose( &configFile );

   if ( isPopulated ) {
      if ( viewParams ) {
         ConfigDisplay();
      }
      else {
         PrintMessage( "Configuration file successfully read\n" );
      }

      return TRUE;
   }
   else {
      return FALSE;
   }
}

Bool LukInitDatabase() {
   if ( runMode != LUK_MODE_SKIP ) {
      const Str *databasePath = ConfigGetValue( "database_path" );
      int dbInitResult;
      dbInitResult = DatabaseInitializeFile( databasePath->value );

      if ( dbInitResult == DB_INIT_SUCCESS ) {
         /* Check whether to save database on every STORE query. */
         const Str *value = ConfigGetValue( "database_save_on_store" );
         if ( runMode != LUK_MODE_SKIP && value != NULL &&
            strcmp( value->value, "true" ) == 0 ) {
            saveDatabaseOnStore = TRUE;
            PrintMessage( "Will save the database after every STORE query\n" );
         }

         return TRUE;
      }
      else if ( dbInitResult == DB_INIT_RECORDS_LOAD_FAILED ) {
         PrintMessage( "   - Will proceed without loading previous data\n" );
         return TRUE;
      }
      else {
         PrintError( "Database failed to initialize\n" );
         return FALSE;
      }
   }
   else {
      PrintMessage( 
         "Running in skip mode. No database file will be loaded or saved\n" );
      /* Initialize a clean database. */
      DatabaseInitialize();
      return TRUE;
   }
}

void LukCloseDatabase() {
   LukSaveDatabase();
   DatabaseShutdown();
}

void LukSaveDatabase() {
   /* We don't need to save the database when in skip mode or it 
      doesn't need to be saved. */
   if ( runMode != LUK_MODE_SKIP && DatabaseIsSaveNeeded() ) {
      const Str *databasePath = ConfigGetValue( "database_path" );
      if ( databasePath != NULL ) {
         DatabaseSave( databasePath->value );
      }
   }
}

Bool LukInitServer() {
   RconResponse initialResponse;
   const Str *serverIpAddress;
   Bool isConnected = FALSE;

   const Str *ipFromConfig = ConfigGetValue( "server_address" );
   const Str *serverPort = ConfigGetValue( "server_port" );
   const Str *serverPassword = ConfigGetValue( "server_password" );

   /* Convert any special IP address names to their corresponding 
      numeric representations. */
   if ( strcmp( ipFromConfig->value, "localhost" ) == 0 ) {
      serverIpAddress = StrNew( "127.0.0.1" );
   }
   else {
      serverIpAddress = StrCopy( ipFromConfig );
   }

   if ( ServerInit( serverIpAddress, serverPort ) ) {
      /* Try to connect to the server a few times if the first time fails. */
      int tries = LUK_SERVER_CONNECTION_RETRIES;
      const int timeout = LUK_SERVER_CONNECTION_WAIT_TIME;
      int loginStatus;

      while ( tries > 0 && ! isConnected && LukIsRunning() ) {
         PrintMessage( "Logging in to RCON server at: %s:%s\n", 
            serverIpAddress->value, serverPort->value );
         loginStatus = 
            ServerLogin( serverPassword, &initialResponse, timeout );
            
         if ( loginStatus == SV_ERR_NONE ) {
            isConnected = TRUE;
         }
         /* Break out of the loop on unrecoverable errors. */
         else if ( loginStatus == SV_ERR_TIMEOUT ) {
            PrintMessage( "   - No reply from RCON server. Retrying...\n" );
            tries -= 1;
         }
         else {
            break;
         }
      }

      if ( isConnected ) {
         PrintMessage( "Successfully logged in to RCON server\n" );
         LukProcessInitialReponse( &initialResponse );
      }
      else {
         PrintError( "Login failed.\n" );
      }

      atexit( LukShutdownServer );
   }
   else {
      PrintError( "Failed to initalize a server connection\n" );
   } 

   StrDel( serverIpAddress );
   return isConnected;
}

Bool LukProcessInitialReponse( const RconResponse *response ) {
   const Byte *body = response->body;

   int serverProtocol;
   const Str *serverHostname;

   int totalUpdates;
   int update;

   const Str *map;

   int player;
   int totalPlayers;

   /* Read server protocol: */
   serverProtocol = *body;
   body += 1;

   /* Find server hostname: */
   serverHostname = StrNew( ( const char * ) body );
   body += serverHostname->length + 1;

   /* Read the updates fields. */
   totalUpdates = *body;
   body += 1;

   for ( update = 0; update < totalUpdates; update += 1 ) {
      int updateType = ( int ) *body;
      body += 1;

      switch ( updateType ) {
         /* We are only concerned with the current map. Ignore the rest. */
         case SVRCU_MAP:
            map = StrNew( ( const char * ) body );
            body += map->length + 1;
 
            break;

         case SVRCU_PLAYERDATA:
            totalPlayers = ( int ) *body;
            body += 1;

            player = 0;
            while ( player < totalPlayers ) {
               if ( *body == 0 ) {
                  player += 1;
               }

               body += 1;
            }

            break;

         case SVRCU_ADMINCOUNT:
            body += 1;
            break;
      }
   }

   PrintMessage( "RCON server: \n" );
   PrintMessage( "   - Protocol: %d\n", serverProtocol );
   PrintMessage( "   - Hostname: %s\n", serverHostname->value );

   DatabaseChangeMap( map );

   StrDel( map );
   StrDel( serverHostname );

   return TRUE;
}

void LukShutdownConfigSystem() {
   ConfigShutdown();
}

/* Function to process server responses. */
void LukProcessResponse( const RconResponse *response ) {
   Str *outputUnclean = NULL;
   Str *output = NULL;
   Str *updateOutput = NULL;

   /* Bail out if we have no data. */
   if ( response->bodyLength <= 0 ) {
      return;
   }

   outputUnclean = StrNewSub( ( const char * ) response->body, 
      response->bodyLength - 1 );
   output = StrTrim( outputUnclean );

   switch ( response->header ) {
      /* Message responses. */
      case SVRC_MESSAGE:
         LukProcessMessageResponse( output );
         break;

      /* Update responses: */
      case SVRC_UPDATE:
         updateOutput = StrNew( output->value + 1 );

         if ( output->value[ 0 ] == SVRCU_MAP ) {
            LukChangeMap( updateOutput );
         }

         StrDel( updateOutput );

         break;
   }

   StrDel( outputUnclean );
   StrDel( output );
}

void LukProcessMessageResponse( const Str *message ) {
   /* Execute the message if it's a valid luk query. */
   if ( QueryIsValidCapsule( message ) && QueryUnpack( message ) ) {
      command_t *command;

      /* Start a fresh reply to the query. */
      ReplyReset();
      ReplySetQueryId( QueryGetId() );

      command = CommandCreate( QueryGetCargo() );
      QueryDeleteCargo();

      if ( command != NULL ) {
         CommandExecute( command );
         CommandDestroy( command );

         /* Save the database if the database needs saving, but only if
            the option is enabled. */
         if ( saveDatabaseOnStore ) {
            LukSaveDatabase();
         }

         /* Only send back a reply if we have any data. */
         if ( ReplyGetDataSize() > 0 ) {
            Str *serverCommand;

            serverCommand = ReplyBuildCommand();
            ServerSendCommand( serverCommand );

            StrDel( serverCommand );
         }
      }
   }
}

void LukChangeMap( const Str *map ) {
   /* Reset the next query ID back to zero. */
   QueryResetId();

   /* Save the database data into a file after every map. */
   LukSaveDatabase();
   DatabaseChangeMap( map );
   LukPrintCurrentMap();
}

void LukPrintCurrentMap( void ) {
   const Str *newMap = DatabaseGetCurrentMap();
   PrintHeader( newMap->value );
}

void LukShutdownServer( void ) {
   ServerShutdown();
}

void LukExit( int sig ) {
   ( void ) sig;
   signal( SIGINT, LukExit );

   #if ! ( defined _WIN32 || defined _WIN64 )
   printf( "\n" );
   #endif
   
   lukIsRunning = FALSE;
}

Bool LukIsRunning( void ) {
   return lukIsRunning; 
}

/* Function to print the help menu. */
void LukPrintHelpMenu( const char *programPath ) {
   /* Small description. */
   printf( 
      "luk is a permanent data storage environment for Skulltag.\n\n" 

      /* Usage information. */
      "Usage: \n"
      "  %s [ options ]\n"
      "  To quit luk when running, press Ctrl+C\n\n", 
      programPath 
   );

   printf(
      /* Command-line options. */
      "Options: \n"
      "  -c <path_to_file>\tSpecify path to a configuration file\n"
      "  -d <map_lump>\t\tDeletes data of <map_lump> from database\n"
      "  -g\t\t\tGenerate a blank configuration file\n"
          "     \t\t\tin present directory\n"
      "  -h\t\t\tDisplay this help menu\n"
      "  -p\t\t\tView loaded configuration parameters\n"
      "  -s\t\t\tSkip mode. Skip the loading and saving\n"
      "  \t\t\tof the database file\n\n"

      /* Credits. */
      "luk was developed by Positron with help from Aiur850, Jroc, "
      "and Frank.\n"

      "Compiled on: " __DATE__ "\n"
   );
}

void LukGenerateNewConf( void ) {
   const char *confTemplate = LUK_COFIG_FILE_TEMPLATE;
   FILE *newConf;

   printf( "Generating a blank configuration file in current directory\n" );

   /* Before creating a new file, make sure there isn't already a 
      configuration file present in the current directory. */
   newConf = fopen( LUK_TEMPLATE_CONFIG_FILE_PATH, "r" );
   if ( newConf != NULL ) {
      printf( "Error: File already exists\n" );
      fclose( newConf );
      return;
   }

   newConf = fopen( LUK_TEMPLATE_CONFIG_FILE_PATH, "w" );
   if ( newConf != NULL ) {
      fwrite( confTemplate, strlen( confTemplate ), 1, newConf );
      fclose( newConf );
   }
   else {
      printf( "Cannot create a new configuration file\n" );
   }
}

Bool LukDeleteMapEntry( void ) {
   const char *arg;
   Str *mapName;

   /* Bail out if no map was given. */
   if ( ( arg = PROGA_NextArg() ) == NULL ) {
      PrintError( "No map name given.\n" );
      return FALSE;
   }

   mapName = StrNew( arg );

   if ( DatabaseDelete( mapName ) ) {
      PrintMessage( "Successfully deleted map entry: %s\n", mapName->value );
   }
   else {
      PrintError( "Failed to locate map entry with name: %s\n",
         mapName->value );
   }

   StrDel( mapName );

   return TRUE;
}
