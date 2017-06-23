#ifndef RCP_TESTS_H
#define RCP_TESTS_H

#include <stdbool.h>

#ifndef _BSD_SOURCE
#define _BSD_SOURCE //This may make usleep work.
#endif

#include <unistd.h> //For sleeping


#define SEND_PACKETS_COUNT_1 500 //Should be no more than 99 digits

void simple_open_bind_close_test(char *otherIP);
void simple_send_packets_test(char *otherIP);
void simple_receive_packets_test(char *otherIP);

#endif
