#include <string.h>
#include <unistd.h> //For close()
#include "rcp.h"
#include "rcp_config.h"

int32_t rcp_socket(){
    // DEBUG_PRINT("Creating UDP socket.\n");
    return (int32_t) socket(AF_INET, SOCK_DGRAM, 0);
}

int32_t rcp_bind(int32_t fd, rcp_sockaddr_in *rcp_sock, uint16_t port){
    struct sockaddr_in myaddr = rcp_sock->addr;

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY); //Let OS decide which IP address this port is assigned to
    myaddr.sin_port = htons(port); //The port that the socket binds to
    // DEBUG_PRINT("Binding socket at %x:%hu\n", myaddr.sin_addr.s_addr, my_addr.sin_port);
    return bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
}

int32_t rcp_close(int32_t fd){
    return close(fd);
}
