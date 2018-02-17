 #define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include "rcp_config.h"
#include "rcp_packet.h"

#include <stdint.h>
#include <stdlib.h>

#include <string.h>

#include <arpa/inet.h> //For htons and related

Packet *createPacket(uint8_t syn, uint8_t ack, uint32_t seq, uint32_t dataSize, uint8_t const *data) {
    Packet *packet = (Packet *)malloc(sizeof(Packet));
    // copy info into packet
    packet->syn = syn;
    packet->ack = ack;
    packet->seq = seq;

    if(data!=NULL){
        //Copy data so the user can do whatever with that data.
        packet->data = malloc(dataSize);
        memcpy(packet->data, data, dataSize);
    } else if(data==NULL && dataSize!=0){
        packet->data = malloc(dataSize); //If data is null and packet size is not 0, then a blank packet with data space is required.
    } else {
        packet->data = NULL; //Otherwise user just wants blank packet with no space for data
    }

    packet->dataSize = dataSize;
    return packet;
}

void *serializePacket(Packet *packet){
    uint8_t *buff = malloc(PACKET_HEAD_SIZE+extractDataSize(packet));
    uint8_t *temp_buff = buff;
    /*
    Copy over packet header
     */

    temp_buff = memcpy(temp_buff, &(packet->syn), sizeof(uint8_t));
    temp_buff+=sizeof(uint8_t);
    temp_buff = memcpy(temp_buff, &(packet->ack), sizeof(uint8_t));
    temp_buff+=sizeof(uint8_t);


    uint32_t seq = htonl(packet->seq);

    temp_buff = memcpy(temp_buff, &seq, sizeof(uint32_t));
    temp_buff+=sizeof(uint32_t);

    uint32_t dataSize = htonl(packet->dataSize);

    temp_buff = memcpy(temp_buff, &dataSize, sizeof(uint32_t));
    temp_buff+=sizeof(uint32_t);

    /*
    Copy over data after packet header
     */
    // void *datapos = (void *)(buff+PACKET_HEAD_SIZE);
    temp_buff = memcpy(temp_buff, extractData(packet), extractDataSize(packet));
    return buff;
}

void deserializePacket(void *serialPacket, Packet *packet){
    uint8_t *buff = (uint8_t *)serialPacket;
    memcpy(&(packet->syn), buff, sizeof(uint8_t));
    buff+=1;
    memcpy(&(packet->ack), buff, sizeof(uint8_t));
    buff+=1;

    memcpy(&(packet->seq), buff, sizeof(uint32_t));
    packet->seq = ntohl(packet->seq);
    buff+=sizeof(uint32_t);

    // packet->dataSize = ntohl(seqpos[1]);
    memcpy(&(packet->dataSize), buff, sizeof(uint32_t));
    packet->dataSize = ntohl(packet->dataSize);
    buff+=sizeof(uint32_t);

    packet->data = malloc(packet->dataSize); //Need space to hold data
    /*
    Copy over data after packet header
     */
    memcpy(packet->data, buff, packet->dataSize);
}

inline uint8_t isAck(Packet *packet) {
    return packet->ack;
}

inline uint8_t isSyn(Packet *packet) {
    return packet->syn;
}

inline uint8_t isSynAck(Packet *packet) {
    return packet->ack && packet->syn;
}

inline uint32_t extractSeq(Packet *packet) {
    return packet->seq;
}

inline uint8_t* extractData(Packet *packet) {
    return packet->data;
}

inline uint32_t extractDataSize(Packet *packet) {
    return packet->dataSize;
}

inline uint8_t isData(Packet *packet) {
    return packet->dataSize > 0;
}

inline void destroyPacketData(Packet *packet){
    free(packet->data);
}

void destroyPacket(Packet *packet) {
    if(packet->data != NULL){
        free(packet->data);
    }
    free(packet);
}

Queue *packetize(const uint8_t *data, uint32_t dataSize, uint32_t dataPerPacket){
    Queue *q = queue_new();

    return q;
}
