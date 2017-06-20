#ifndef RCP_PACKET_H
#define RCP_PACKET_H

#include <stdbool.h>
#include <stdint.h>
#include "rcp_queue.h"

typedef struct {
    bool syn;
    bool ack;
    uint32_t seq;
    uint32_t dataSize;
    char* data;
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
Packet* createPacket(bool syn, bool ack, uint32_t seq, uint32_t dataSize, const char *data);

/**
 * true iff packet contains an ack
 * @param  packet
 * @return        true/false
 */
bool isAck(Packet* packet);

/**
 * true iff packet contains a syn
 * @param  packet
 * @return        true/false
 */
bool isSyn(Packet* packet);

/**
 * true iff packet contains an ack
 * @param  packet
 * @return        true/false
 */
bool isSynAck(Packet* packet);

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
char* extractData(Packet* packet);

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
bool isData(Packet* packet);


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
Queue *packetize(const char *data, uint32_t dataSize, uint32_t dataPerPacket);

#endif
