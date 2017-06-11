
#include "rcp_packet.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Packet* createPacket(bool syn, bool ack, uint32_t seq, uint32_t dataSize, const char* data) {
    Packet* packet = (Packet *)malloc(sizeof(Packet));
    // copy info into packet
    packet->syn = syn;
    packet->ack = ack;
    packet->seq = seq;

    //Copy data so the user can do whatever with that data.
    packet->data = malloc(dataSize);
    memcpy(packet->data, data, dataSize);

    packet->dataSize = dataSize;
    return packet;
}

bool isAck(Packet* packet) {
    return packet->ack;
}

bool isSyn(Packet* packet) {
    return packet->syn;
}

bool isSynAck(Packet* packet) {
    return packet->ack && packet->syn;
}

uint32_t extractSeq(Packet* packet) {
    return packet->seq;
}

char* extractData(Packet* packet) {
    return packet->data;
}

uint32_t extractDataSize(Packet* packet) {
    return packet->dataSize;
}

bool isData(Packet* packet) {
    return packet->dataSize > 0;
}

void destroyPacket(Packet* packet) {
    free(packet->data);
    free(packet);
}

Queue *packetize(const char *data, uint32_t dataSize, uint32_t dataPerPacket){
    Queue *q = queue_new();

    return q;
}
