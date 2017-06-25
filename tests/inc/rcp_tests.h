#ifndef RCP_TESTS_H
#define RCP_TESTS_H

#include <stdbool.h>

#ifndef _BSD_SOURCE
#define _BSD_SOURCE //This may make usleep work.
#endif

#include <unistd.h> //For sleeping


#define SEND_PACKETS_COUNT_1 500 //Should be no more than 99 digits
#define DO_PACKET_STATS_TEST 0 //To run receive packet test indefinetly for statistics


void simple_open_bind_close_test(char *otherIP);

#if DO_PACKET_STATS_TEST
void simple_send_packets_test(char *otherIP);
void simple_receive_packets_test(char *otherIP);
#endif

void simple_connect_test(char *otherIP);
void simple_listen_test(char *otherIP);

#endif
