#include <stdio.h>


char* createPacket(int syn, int ack, int seq, int dataSize, char[] data) {
    char flagByte;
    if (syn && ack) {
        flagByte = 3;
    } else if (syn) {
        flagByte = 2;
    } else if (ack) {
        flagByte = 1;
    } else {
        flagByte = 0;
    }
    char* packet = malloc(sizeof(char) + sizeof(int) + sizeof(int) + dataSize);
    packet[0] = flagByte;

    char* pSeq = (char*) &seq;
    copyInt(packet, 1, pSeq);

    char* pDS = (char*) &dataSize;
    copyInt(packet, 1 + sizeof(int), pDS);

    for (int i = 0; i < dataSize; i++) {
        packet[i + 1 + sizeof(int)*2] = data[i]
    }
    return packet;
}

void copyInt(char* pChar, int startingCharInd, char* pInt) {
    for (int i = 0; i < sizeof(int); i++) {
        pChar[i+startingCharInd] = pInt[i];
    }
}
