
#include "rcp_packet.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h> //For htons and related

Packet *createPacket(bool syn, bool ack, uint32_t seq, uint32_t dataSize, const char* data) {
    Packet *packet = (Packet *)malloc(sizeof(Packet));
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

void *serializePacket(Packet *packet){
    char *buff = malloc(PACKET_HEAD_SIZE+extractDataSize(packet));

    /*
    Copy over packet header
     */
    buff[0] = packet->syn;
    buff[1] = packet->ack;
    uint32_t *seqpos = (uint32_t *)(buff+2);
    seqpos[0] = htonl(packet->seq);
    seqpos[1] = htonl(packet->dataSize);

    /*
    Copy over data after packet header
     */
    void *datapos = (void *)(buff+PACKET_HEAD_SIZE);
    memcpy(datapos, extractData(packet), extractDataSize(packet));
    return buff;
}

void deserializePacket(void *serialPacket, Packet *packet){
    char *buff = (char *)serialPacket;
    packet->syn = buff[0];
    packet->ack = buff[1];
    uint32_t *seqpos = (uint32_t *)(buff+2);
    packet->seq = ntohl(seqpos[0]);
    packet->dataSize = ntohl(seqpos[1]);
    packet->data = malloc(packet->dataSize); //Need space to hold data
    /*
    Copy over data after packet header
     */
    void *datapos = (void *)(buff+PACKET_HEAD_SIZE);
    memcpy(packet->data, datapos, extractDataSize(packet));
}

bool isAck(Packet *packet) {
    return packet->ack;
}

bool isSyn(Packet *packet) {
    return packet->syn;
}

bool isSynAck(Packet *packet) {
    return packet->ack && packet->syn;
}

uint32_t extractSeq(Packet *packet) {
    return packet->seq;
}

char* extractData(Packet *packet) {
    return packet->data;
}

uint32_t extractDataSize(Packet *packet) {
    return packet->dataSize;
}

bool isData(Packet *packet) {
    return packet->dataSize > 0;
}

void destroyPacketData(Packet *packet){
    free(packet->data);
}

void destroyPacket(Packet *packet) {
    free(packet->data);
    free(packet);
}

Queue *packetize(const char *data, uint32_t dataSize, uint32_t dataPerPacket){
    Queue *q = queue_new();

    return q;
}
