#include "rcp_config.h"
#include "rcp_packet_tests.h"
#include "rcp_packet.h"


#include <stdio.h>
#include <stdbool.h>

int main() {

    #if !DEBUG
    printf("Debug flag must be set to run tests in rcp_config.h\n");
    return -1;
    #endif

    char data[5] = {'a', 'a', 'a', 'a', 'a'};
    Packet* packet = createPacket(1, 0, 30, 5, data);
    bool ack = isAck(packet);
    bool syn = isSyn(packet);
    //bool synack = isSynAck(packet);
    if (ack) {
        DEBUG_PRINT("is ack\n");
    }
    if (!syn) {
        DEBUG_PRINT("isn't syn\n");
    }
    DEBUG_PRINT("dataSize is %i\n", extractDataSize(packet));
    DEBUG_PRINT("seq num is %i\n", extractSeq(packet));

    DEBUG_PRINT("This is a test\n");

    destroyPacket(packet);
    return 0;
}
