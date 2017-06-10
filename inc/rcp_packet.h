#ifndef RCP_PACKET_H
#define RCP_PACKET_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool syn;
    bool ack;
    uint32_t seq;
    uint32_t dataSize;
    char* data;
} Packet;

Packet* createPacket(bool syn, bool ack, uint32_t seq, uint32_t dataSize, char data[]);

bool isAck(Packet* packet);

bool isSyn(Packet* packet);

bool isSynAck(Packet* packet);

uint32_t extractSeq(Packet* packet);

char* extractData(Packet* packet);

uint32_t extractDataSize(Packet* packet);

bool isData(Packet* packet);

void destroyPacket(Packet* packet);


#endif
