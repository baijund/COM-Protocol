#include <string.h>
#include <unistd.h> //For close()
#include "rcp_config.h"
#include "rcp.h"
#include "rcp_packet.h"

typedef enum {
    PKT_NO_ERROR,
    PKT_SEND_PACKET_ERROR,
    PKT_RECEIVE_PACKET_ERROR
} Packet_Error;

static Packet_Error rcp_send_packet(rcp_connection *rcp_conn, Packet *packet);
static Packet_Error rcp_receive_packet(rcp_connection *rcp_conn, Packet *packet);

int32_t rcp_socket(){
    DEBUG_PRINT("Creating UDP socket.\n");
    return (int32_t) socket(AF_INET, SOCK_DGRAM, 0);
}

int32_t rcp_bind(int32_t fd, rcp_sockaddr_in *rcp_sock, uint16_t port){
    struct sockaddr_in myaddr = rcp_sock->addr;
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY); //Let OS decide which IP address this port is assigned to
    myaddr.sin_port = htons(port); //The port that the socket binds to
    DEBUG_PRINT("Binding socket at %x:%hu\n", ntohl(myaddr.sin_addr.s_addr), ntohs(myaddr.sin_port));
    int32_t b = bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
    return b;
}


RCP_Send_Error rcp_send(rcp_connection *rcp_conn, const void *buf, size_t len){
    return SEND_NO_ERROR;
}


int32_t rcp_close(int32_t fd){
    return close(fd);
}


static Packet_Error rcp_send_packet(rcp_connection *rcp_conn, Packet *packet){
    return PKT_NO_ERROR;
}

static Packet_Error rcp_receive_packet(rcp_connection *rcp_conn, Packet *packet){
    return PKT_NO_ERROR;
}
