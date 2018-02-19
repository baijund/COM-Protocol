#include "rcp_tests.h"
#include "rcp.h"
#include "rcp_packet.h"
#include <assert.h>
#include "rcp_config.h"
#include <stdio.h>
#include <string.h>

#define DECORATE DEBUG_PRINT("-------------\n")

#define TEST_STR_1 "This is a test" //Don't change unless you account for buffer overflow

static int32_t uDelay = 0;

static bool groundMode;
int main(int32_t argc, char **argv){

    #if !DEBUG
    printf("Debug flag must be set to run tests in rcp_config.h\n");
    return -1;
    #endif

    if(argc<4){
        DECORATE;
        DEBUG_PRINT("Usage: rcp_tests s/g otherIP delay\n");
        DEBUG_PRINT("s if this machine goes in space. g if it stays on ground\n");
        DEBUG_PRINT("otherIP is the IP of the other machine\n");
        DECORATE;
        return -1;
    }

    uDelay = atoi(argv[3]);
    DEBUG_PRINT("Delay set to %d MICROSECONDS\n", uDelay);

    groundMode = (strcmp(argv[1],"g")==0);


    if(groundMode){
        DEBUG_PRINT("In ground mode.\n");
    } else{
        DEBUG_PRINT("In space mode\n");
    }

    char *otherIP = argv[2];


    // DECORATE;
    // DEBUG_PRINT("Testing open, bind, and close...\n");
    // simple_open_bind_close_test(otherIP);
    // DEBUG_PRINT("Passed test\n");
    //
    #if DO_PACKET_STATS_TEST
    DECORATE;
    if(groundMode){
        DEBUG_PRINT("Testing destination setup and send packets...\n");
        DEBUG_PRINT("Let server test get to this point and hit enter.\n");
        simple_send_packets_test(otherIP);
        return 0;
    } else {
        DEBUG_PRINT("Testing destination setup and receive packets...\n");
        simple_receive_packets_test(otherIP);
    }
    #endif
    //
    // DECORATE;
    // DEBUG_PRINT("Testing connect and listen.\n");
    // if(groundMode){
    //     simple_connect_test(otherIP);
    // } else {
    //     simple_listen_test(otherIP);
    // }
    //
    // DECORATE;
    // DEBUG_PRINT("Simple testing Send and Receive\n");
    char *simpleBuff = "hello";
    // if(groundMode){
    //     simple_send_test(otherIP, (uint8_t *)simpleBuff, strlen(simpleBuff)+1);
    // } else {
    //     simple_receive_test(otherIP, (uint8_t *)simpleBuff, strlen(simpleBuff)+1);
    // }

    DECORATE;
    DEBUG_PRINT("Extensive testing Send and Receive\n");
    uint32_t buflen = RCP_MAX_PACKET_DATA_SIZE*10; //To send 10 packets
    simpleBuff = malloc(buflen);
    for(int i=0;i<buflen;i++){
        simpleBuff[i] = 'A'+(i%26);
    }
    if(groundMode){
        simple_send_test(otherIP, (uint8_t *)simpleBuff, buflen);
    } else {
        simple_receive_test(otherIP, (uint8_t *)simpleBuff, buflen);
    }
    free(simpleBuff);

    DECORATE;
    DEBUG_PRINT("All RCP tests passed\n");
    return 0;
}

void simple_open_bind_close_test(char *otherIP){

    rcp_connection rcp_conn = rcp_initConnection();

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    if(groundMode){
        assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, GROUND_PORT)==0);
    } else {
        assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, SPACE_PORT)==0);
    }
    DEBUG_PRINT("Bind passed\n");

    assert(rcp_close(&rcp_conn)==0);
    DEBUG_PRINT("Close passed\n");
}

#if DO_PACKET_STATS_TEST
void simple_send_packets_test(char *otherIP){
    rcp_connection rcp_conn = rcp_initConnection();

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, GROUND_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, SPACE_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    DEBUG_PRINT("Attempting to send %d packets\n", SEND_PACKETS_COUNT_1);
    for(int32_t i=0; i<SEND_PACKETS_COUNT_1; i++){
        uint32_t contentLength = strlen(TEST_STR_1)+100;
        uint8_t *content = calloc(1, contentLength); //Can append upto 99 more char
        sprintf(content, "%s_%d", TEST_STR_1, i);
        Packet *packet = createPacket(false, false, 0, contentLength, content);
        free(content); //No need for content after creating packet
        assert(rcp_send_packet(&rcp_conn, packet) == PACKET_SERIAL_SIZE(packet));
        DEBUG_PRINT("Sent packet %d of %ld bytes with dataSize %d bytes and content %s\n", i, PACKET_SERIAL_SIZE(packet), packet->dataSize, packet->data);
        destroyPacket(packet);
        usleep(uDelay); //Minimum delay between each packet send
    }
    DEBUG_PRINT("Sent %d packets\n", SEND_PACKETS_COUNT_1);

    assert(rcp_close(&rcp_conn)==0);
    DEBUG_PRINT("Close passed\n");
}

void simple_receive_packets_test(char *otherIP){
    rcp_connection rcp_conn = rcp_initConnection();

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, SPACE_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, GROUND_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    int32_t count = 0;
    DEBUG_PRINT("Attempting to receive packets\n");
    for(;;){
        Packet packet;
        struct timeval tv;
        tv.tv_sec = 1; //Basically the sleep(1) for the forever loop
        tv.tv_usec = 0;
        int32_t recret = rcp_receive_packet(&rcp_conn, &packet, tv);
        if(recret<0){
            //DEBUG_PRINT("Socket timed out.\n");
            continue;
        }
        count++;
        DEBUG_PRINT("Received %d packets. This one with dataSize %d and content %s\n",count,packet.dataSize, packet.data);
        // DEBUG_PRINT("Content Hex: \n");
        // for(int i=0;i<packet.dataSize;i++){
        //     DEBUG_PRINT("0x%x ", packet.data[i]);
        // }
        // DEBUG_PRINT("\n");
        destroyPacketData(&packet);
    }
    DECORATE;
    DEBUG_PRINT("Received %d packets\n", count);

    assert(rcp_close(&rcp_conn)==0);
    DEBUG_PRINT("Close passed\n");
}
#endif


void simple_connect_test(char *otherIP){
    rcp_connection rcp_conn = rcp_initConnection();

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, SPACE_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, GROUND_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    assert(rcp_connect(&rcp_conn)==RCP_NO_ERROR);
    DEBUG_PRINT("Connect finished\n");

    DEBUG_PRINT("Attempting to close\n");
    assert(rcp_close(&rcp_conn)==0);
    DEBUG_PRINT("Close passed\n");
}

void simple_listen_test(char *otherIP){
    rcp_connection rcp_conn = rcp_initConnection();

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, GROUND_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, SPACE_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    assert(rcp_listen(&rcp_conn)==RCP_NO_ERROR);
    DEBUG_PRINT("Listen finished\n");

    assert(rcp_close(&rcp_conn)==0);
    DEBUG_PRINT("Close passed\n");
}


void simple_send_test(char *otherIP, uint8_t *simpleBuff, uint32_t len){
    rcp_connection rcp_conn = rcp_initConnection();

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, SPACE_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, GROUND_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    assert(rcp_connect(&rcp_conn)==RCP_NO_ERROR);
    DEBUG_PRINT("Connect finished\n");

    DEBUG_PRINT("Sending %d bytes\n", len);
    DEBUG_PRINT("Attempting to send:\n");
    for(int i=0;i<len;i++){
        DEBUG_PRINT("%c", simpleBuff[i]);
    }
    DEBUG_PRINT("\n");
    rcp_send(&rcp_conn, simpleBuff, len);
    DEBUG_PRINT("Send Finished\n");

    assert(rcp_close(&rcp_conn)==0);
    DEBUG_PRINT("Close passed\n");
}

void simple_receive_test(char *otherIP, uint8_t *simpleBuff, uint32_t len){
    rcp_connection rcp_conn = rcp_initConnection();

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, otherIP, GROUND_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, SPACE_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    assert(rcp_listen(&rcp_conn)==RCP_NO_ERROR);
    DEBUG_PRINT("Listen finished\n");

    uint8_t *buf = malloc(len);
    uint32_t bytesRead;
    struct timeval to;
    DEBUG_PRINT("Attempting to receive buffer of size %d.\n", len);
    rcp_receive(&rcp_conn, buf, len, &bytesRead, to);
    assert(bytesRead==len);
    DEBUG_PRINT("Receive finished\n");
    // DEBUG_PRINT("Should have received:\n");
    // for(int i=0;i<len;i++){
    //     DEBUG_PRINT("0x%x ", simpleBuff[i]);
    // }
    // DEBUG_PRINT("\n");
    // DEBUG_PRINT("Received actually:\n");
    // for(int i=0;i<bytesRead;i++){
    //     DEBUG_PRINT("0x%x ", buf[i]);
    // }
    // DEBUG_PRINT("\nComparison:\n");
    // for(int i=0;i<bytesRead;i++){
    //     DEBUG_PRINT("%d", buf[i]==simpleBuff[i]);
    // }
    // DEBUG_PRINT("\n");
    assert(memcmp(buf, simpleBuff, len)==0);
    DEBUG_PRINT("Received the correct data.\n");
    free(buf);

    assert(rcp_close(&rcp_conn)==0);
    DEBUG_PRINT("Close passed\n");
}
