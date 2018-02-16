#ifndef RCP_PACKET_H
#define RCP_PACKET_H

#include <stdint.h>
#include "rcp_queue.h"

/*
The size of a packet w/o the pointer for transfer.
The pointer is useless after transfer, so it is not necessary.
*/
#define PACKET_HEAD_SIZE sizeof(uint8_t)*2+sizeof(uint32_t)*2


#define PACKET_SERIAL_SIZE(p_packet) (extractDataSize(p_packet)+PACKET_HEAD_SIZE)

typedef struct {
    uint8_t syn;
    uint8_t ack;
    uint32_t seq;
    uint32_t dataSize;
    uint8_t* data;
} Packet;

/**
 * Creates a packet
 * @param  syn      true if packet should contain a syn
 * @param  ack      true if packet should contain an ack
 * @param  seq      sequence number
 * @param  dataSize size of data
 * @param  data     data
 * @return          a packet
 */
Packet* createPacket(uint8_t syn, uint8_t ack, uint32_t seq, uint32_t dataSize, uint8_t const *data);

/**
 * Serializes a packet into a single buffer
 * The buffer needs to be freed after
 * @param  packet Pointer to Packet to be serialized
 * @return        Buffer containing serialized packet
 */
void *serializePacket(Packet *packet);

/**
 * Creates a new packet from serialized data
 * Function will allocate space for data.
 * Packet must be destroyed after use.
 * @param  serialPacket serialized data
 * @param  packet       pointer to packet to hold contents
 */
void deserializePacket(void *serialPacket, Packet *packet);

/**
 * true iff packet contains an ack
 * @param  packet
 * @return        true/false
 */
uint8_t isAck(Packet* packet);

/**
 * true iff packet contains a syn
 * @param  packet
 * @return        true/false
 */
uint8_t isSyn(Packet* packet);

/**
 * true iff packet contains an ack
 * @param  packet
 * @return        true/false
 */
uint8_t isSynAck(Packet* packet);

/**
 * Gets the sequence number of a packet
 * @param  packet
 * @return        sequence number
 */
uint32_t extractSeq(Packet* packet);

/**
 * Gets the data from the packet
 * @param  packet
 * @return        data
 */
uint8_t* extractData(Packet* packet);

/**
 * Gets the size of the data in the packet
 * @param  packet
 * @return        data size
 */
uint32_t extractDataSize(Packet* packet);

/**
 * True iff packet is a data packet
 * @param  packet
 * @return        true/false
 */
uint8_t isData(Packet* packet);


/**
 * Frees data inside of packet.
 * Usefil for statically allocated packets.
 * @param packet packet who's data needs to be freed
 */
void destroyPacketData(Packet *packet);

/**
 * Deallocates space for packet and data
 * @param packet
 */
void destroyPacket(Packet* packet);

/**
 * Packetizes a data buffer (allocates data)
 * @param  data          buffer to be packetized
 * @param  dataSize      size of buffer
 * @param  dataPerPacket amount of data in each packet in bytes
 * @return               Pointer to queue of packets
 */
Queue *packetize(const uint8_t *data, uint32_t dataSize, uint32_t dataPerPacket);

#endif
