#ifndef RCP_CONFIG_H
#define RCP_CONFIG_H

#include <stdlib.h>

#ifndef DEBUG
#define DEBUG 1 //Set this flag while debugging
#endif

#if DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false ) //Only works on C99 compiler
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#define NDEBUG 1 //This is for preventing asserts while debugging.
#endif

#define GROUND_PORT 30332
#define SPACE_PORT 30313

#endif