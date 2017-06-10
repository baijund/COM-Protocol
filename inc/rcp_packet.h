#ifndef RCP_PACKET_H
#define RCP_PACKET_H

#include <stdbool.h>

typedef struct {
    bool syn;
    bool ack;
    int seq;
    int dataSize;
    char* data;
} Packet;

Packet* createPacket(bool syn, bool ack, int seq, int dataSize, char data[]);

bool isAck(Packet* packet);

bool isSyn(Packet* packet);

bool isSynAck(Packet* packet);

int extractSeq(Packet* packet);

char* extractData(Packet* packet);

int extractDataSize(Packet* packet);

bool isData(Packet* packet);

void destroyPacket(Packet* packet);


#endif
