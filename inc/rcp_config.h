#ifndef RCP_CONFIG_H
#define RCP_CONFIG_H

#include <stdint.h> //For portable types
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h> //For struct tv

#ifndef COM_DEBUG
#define COM_DEBUG 1 //Set this flag while debugging
#endif

#if COM_DEBUG
#include <stdio.h>

#define COM_DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false ) //Only works on C99 compiler
#else
#define COM_DEBUG_PRINT(...) do{ } while ( false )
#define NCOM_DEBUG 1 //This is for preventing asserts while debugging.
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

#define TRANSMISSION_SPEED 9600.0 //bps
#define PREAMBLE_LENGTH 24 //bits
#define POSTAMBLE_LENGTH 14 //bits
#define AX_25_AND_UDP_OVERHEAD (1+16+20+8)*8 //bits


#define RCP_MAX_PACKET_DATA_SIZE 1024 //Max packet length (bytes)

#define PACKET_TRANSMISSION_TIME (((RCP_MAX_PACKET_DATA_SIZE*8)+PREAMBLE_LENGTH+POSTAMBLE_LENGTH+AX_25_AND_UDP_OVERHEAD)/TRANSMISSION_SPEED)

#define RCP_STATE_RCVD_SYN_TO_SEC 60 //(INTEGER) The state timeout in seconds
#define RCP_STATE_RCVD_SYN_TO_USEC 0 //(INTEGER) The state timeout in microseconds



#define RCP_ESTABLISHED_SERVER_TO_SEC 1
#define RCP_ESTABLISHED_SERVER_TO_USEC 1e3 //This should probably be max round trip time


#define RCP_SWITCH_US 0 //TODO Set to whatever the observed best is.
#define RCP_SWITCH_SEC 1

#define RCP_SYN_SENT_TO_SEC 2*RCP_SWITCH_SEC
#define RCP_SYN_SENT_TO_USEC 2*RCP_SWITCH_US
#define RCP_RCVD_SYN_TO_SEC 2*RCP_SWITCH_SEC //(INTEGER) The timeout in seconds
#define RCP_RCVD_SYN_TO_USEC 2*RCP_SWITCH_US //(INTEGER) The timeout in microseconds
#define RCP_STATE_SYN_SENT_TO_SEC 60
#define RCP_STATE_SYN_SENT_TO_USEC 0

#define RCP_SLIDING_WINDOW_LEN 32
#define RCP_SERVER_RETRIES 3

//-----------------------------------------------
//-----------------------------------------------

#endif
