#ifndef RCP_CONFIG_H
#define RCP_CONFIG_H

#ifndef DEBUG

#define DEBUG 1 //Set this flag while debugging
#endif

#if DEBUG
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false ) //Only works on C99 compiler

// //These will change depending on actual IPs for debugging.
// #define DEBUG_SPACE_IP "172.17.0.3"
// #define DEBUG_GROUND_IP "172.17.0.2"

#else
#define DEBUG_PRINT(...) do{ } while ( false )
#define NDEBUG 1 //This is for preventing asserts while debugging.
#endif

#define GROUND_PORT 30332
#define SPACE_PORT 30313

#endif
