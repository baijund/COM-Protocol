#include "rcp_tests.h"
#include "rcp.h"
#include "rcp_packet.h"
#include <assert.h>
#include "rcp_config.h"
#include <stdio.h>
#include <string.h>

#define DECORATE DEBUG_PRINT("-------------\n")

#define TEST_STR_1 "This is a test"

int main(int32_t argc, char **argv){

    #if !DEBUG
    printf("Debug flag must be set to run tests in rcp_config.h\n");
    return -1;
    #endif

    if(argc<3){
        DEBUG_PRINT("Usage: rcp_tests s/g otherIP\n");
        return -1;
    }

    bool groundMode = (strcmp(argv[1],"g")==0);

    if(groundMode){
        DEBUG_PRINT("In ground mode.\n");
    } else{
        DEBUG_PRINT("In space mode\n");
    }

    char *otherIP = argv[2];


    DECORATE;
    DEBUG_PRINT("Testing open, bind, and close...\n");
    simple_open_bind_close_test(otherIP);
    DEBUG_PRINT("Passed test\n");
    DECORATE;
    if(groundMode){
        DEBUG_PRINT("Testing destination setup and send packets...\n");
        DEBUG_PRINT("Let server test get to this point and hit enter.\n");
        simple_send_packets_test(otherIP);
    } else {
        DEBUG_PRINT("Testing destination setup and receive packets...\n");
        simple_receive_packets_test(otherIP);
    }

    DECORATE;
    DEBUG_PRINT("All RCP tests passed\n");
    return 0;
}

void simple_open_bind_close_test(char *otherIP){

    rcp_connection rcp_conn;

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, GROUND_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    assert(rcp_close(fd)==0);
    DEBUG_PRINT("Close passed\n");
}

void simple_send_packets_test(char *otherIP){
    rcp_connection rcp_conn;

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, GROUND_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, SPACE_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    DEBUG_PRINT("Attempting to send 50 packets\n");
    for(int32_t i=0; i<50; i++){
        uint32_t contentLength = strlen(TEST_STR_1)+10;
        char *content = calloc(1, contentLength); //Can append upto 9 more char
        sprintf(content, "%s_%d", TEST_STR_1, i);
        Packet *packet = createPacket(false, false, 0, contentLength, content);
        free(content); //No need for content after creating packet
        assert(rcp_send_packet(&rcp_conn, packet) == PACKET_SERIAL_SIZE(packet));
        DEBUG_PRINT("Sent packet %d of %ld bytes and content %s\n", i, PACKET_SERIAL_SIZE(packet), packet->data);
        destroyPacket(packet);
    }
    DEBUG_PRINT("Sent 50 packets\n");

    assert(rcp_close(fd)==0);
    DEBUG_PRINT("Close passed\n");
}

void simple_receive_packets_test(char *otherIP){
    rcp_connection rcp_conn;

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, SPACE_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, GROUND_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    DEBUG_PRINT("Attempting to receive 50 packets\n");
    for(int32_t i=0; i<50; i++){
        Packet packet;
        rcp_receive_packet(&rcp_conn, &packet);
        DEBUG_PRINT("Received packet %d with dataSize %d and content %s\n",i,packet.dataSize, packet.data);
        destroyPacketData(&packet);
    }
    DEBUG_PRINT("Received 50 packets\n");

    assert(rcp_close(fd)==0);
    DEBUG_PRINT("Close passed\n");
}
