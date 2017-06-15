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

    DEBUG_PRINT("All RCP tests passed\n");
    return 0;
}

void simple_open_bind_close_test(){
    int32_t fd = rcp_socket();
    assert(fd>=0);
    DEBUG_PRINT("Create socket passed\n");

    rcp_sockaddr_in rcp_addr;
    assert(rcp_bind(fd, &rcp_addr, GROUND_PORT)==0);
    DEBUG_PRINT("Bind passed\n");

    assert(rcp_close(fd)==0);
    DEBUG_PRINT("Close passed\n");
}
