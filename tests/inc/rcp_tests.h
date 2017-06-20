#ifndef RCP_TESTS_H
#define RCP_TESTS_H

#include <stdbool.h>


void simple_open_bind_close_test(char *otherIP);
void simple_send_packets_test(char *otherIP);
void simple_receive_packets_test(char *otherIP);

#endif
