#include "config.h"
#include "rcp_packet_tests.h"
#include "rcp_packet.h"


#include <stdio.h>
#include <stdbool.h>

int main() {
    char data[5] = {'a', 'a', 'a', 'a', 'a'};
    Packet* packet = createPacket(1, 0, 30, 5, data);
    bool ack = isAck(packet);
    bool syn = isSyn(packet);
    //bool synack = isSynAck(packet);
    if (ack) {
        printf("is ack\n");
    }
    if (!syn) {
        printf("isn't syn\n");
    }
    printf("dataSize is %i\n", extractDataSize(packet));
    printf("seq num is %i\n", extractSeq(packet));

    DEBUG_PRINT("This is a test\n");
    return 0;
}
