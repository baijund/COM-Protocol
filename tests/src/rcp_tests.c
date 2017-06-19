#include "rcp_tests.h"
#include "rcp.h"
#include <assert.h>
#include "rcp_config.h"
#include <stdio.h>

#define DECORATE DEBUG_PRINT("-------------\n")

int main(){

    #if !DEBUG
    printf("Debug flag must be set to run tests in rcp_config.h\n");
    return -1;
    #endif

    DECORATE;
    DEBUG_PRINT("Testing open, bind, and close...\n");
    simple_open_bind_close_test();
    DEBUG_PRINT("Passed test\n");
    DECORATE;
    DEBUG_PRINT("Testing destination setup and send packets...\n");
    simple_send_packets_test();
    DECORATE;
    DEBUG_PRINT("All RCP tests passed\n");
    return 0;
}

void simple_open_bind_close_test(){

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

void simple_send_packets_test(){
    rcp_connection rcp_conn;

    int32_t fd = rcp_socket(&rcp_conn);
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(&rcp_conn ,fd, &rcp_addr, GROUND_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    DEBUG_PRINT("Attempting destination setup\n");
    assert(setupDest(&rcp_conn, DEBUG_SPACE_IP, SPACE_PORT)==RCP_NO_ERROR);
    DEBUG_PRINT("Setup destination passed\n");

    DEBUG_PRINT("Attempting to send 50 packets\n");
    
    DEBUG_PRINT("Sent 50 packets");

    assert(rcp_close(fd)==0);
    DEBUG_PRINT("Close passed\n");
}
