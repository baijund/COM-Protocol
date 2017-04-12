#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char bool;

typedef struct {
    bool syn;
    bool ack;
    int seq;
    int dataSize;
    char* data;
} Packet;

Packet* createPacket(bool syn, bool ack, int seq, int dataSize, char data[]) {
    Packet* packet = (Packet *)malloc(sizeof(Packet));
    // copy info into packet
    packet->syn = syn;
    packet->ack = ack;
    packet->seq = seq;
    packet->dataSize = dataSize;
    packet->data = data;
    return packet;
}

bool isAck(Packet* packet) {
    return packet->ack;
}

bool isSyn(Packet* packet) {
    return packet->syn;
}

bool isSynAck(Packet* packet) {
    if (packet->ack && packet->syn) {
        return 1;
    }
    return 0;
}

int extractSeq(Packet* packet) {
    return packet->seq;
}

char* extractData(Packet* packet) {
    return packet->data;
}

int extractDataSize(Packet* packet) {
    return packet->dataSize;
}

bool isData(Packet* packet) {
    return packet->dataSize > 0;
}

void destroyPacket(Packet* packet) {
    free(packet->data);
    free(packet);
}

// Packet** packetize(int startingSeqNum, char* data) {
//
// }

int main() {
    char data[5] = {'a', 'a', 'a', 'a', 'a'};
    Packet* packet = createPacket(1, 0, 30, 5, data);
    bool ack = isAck(packet);
    bool syn = isSyn(packet);
    bool synack = isSynAck(packet);
    if (ack) {
        printf("is ack\n");
    }
    if (!syn) {
        printf("isn't syn\n");
    }
    printf("dataSize is %i\n", extractDataSize(packet));
    printf("seq num is %i\n", extractSeq(packet));
}
