#ifndef RCP_CONFIG_H
#define RCP_CONFIG_H

#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h> //For struct tv

#ifndef DEBUG
#define DEBUG 1 //Set this flag while debugging
#endif

#if DEBUG
#include <stdio.h>

#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false ) //Only works on C99 compiler
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#define NDEBUG 1 //This is for preventing asserts while debugging.
#endif

//----------------------------------------------
//Ports
//----------------------------------------------

#define GROUND_PORT 30332
#define SPACE_PORT 30313

//-----------------------------------------------
//-----------------------------------------------

//----------------------------------------------
//Timeouts
//----------------------------------------------

//THESE ARE ONLY DEFAULT VALUES. Do not modify if not observed optimal.
//Can set timouts programatically.
//If any timeout is set to 0, the state waits indefinetly.

#define RCP_RCVD_SYN_TO_SEC 0 //(INTEGER) The timeout in seconds
#define RCP_RCVD_SYN_TO_USEC 500e3 //(INTEGER) The timeout in microseconds

#define RCP_SYN_SENT_TO_SEC 0
#define RCP_SYN_SENT_TO_USEC 500e3

//-----------------------------------------------
//-----------------------------------------------

#endif
