#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

#define DEBUG 1 //Set this flag while debugging

#if DEBUG

#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false ) //Only works on C99 compiler

#else

#define DEBUG_PRINT(...) do{ } while ( false )

#endif


#endif
