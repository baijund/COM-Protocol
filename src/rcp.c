#include <string.h>
#include <unistd.h> //For close()
#include <netdb.h> //For host resolution
#include "rcp_config.h"
#include "rcp.h"

int32_t rcp_socket(rcp_connection *rcp_conn){
    DEBUG_PRINT("Creating UDP socket.\n");
    rcp_conn->fd = (int32_t) socket(AF_INET, SOCK_DGRAM, 0);
    return rcp_conn->fd;
}

int32_t rcp_bind(rcp_connection *rcp_conn, int32_t fd, rcp_sockaddr_in *rcp_sock, uint16_t port){
    struct sockaddr_in myaddr = *rcp_sock;
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY); //Let OS decide which IP address this port is assigned to
    myaddr.sin_port = htons(port); //The port that the socket binds to
    DEBUG_PRINT("Binding socket at %x:%hu\n", ntohl(myaddr.sin_addr.s_addr),
     ntohs(myaddr.sin_port));
    int32_t b = bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
    if(b==0){
        DEBUG_PRINT("Successful bind\n");
        rcp_conn->this_addr = myaddr; //Copies over the address information over to the connection if successful bind
    } else{
        DEBUG_PRINT("Unsuccessful bind\n");
    }
    return b;
}


RCP_Error setupDest(rcp_connection *conn, const char *host, uint16_t port){
    struct hostent *hp;     /* host information */
    struct sockaddr_in destaddr;    /* dest address */

    /* fill in the dest's address and data */
    memset((char*)&destaddr, 0, sizeof(struct sockaddr_in));
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(port);

    /* look up the address of the server given its name */
    hp = gethostbyname(host);
    if (!hp) {
        DEBUG_PRINT("Could not obtain address of %s\n", host);
    	return RCP_HOSTNAME_FAIL;
    }
    /* put the host's address into the server address structure */
    memcpy((void *)&destaddr.sin_addr, hp->h_addr_list[0], hp->h_length);
    conn->dest_addr = destaddr;
    DEBUG_PRINT("Successfully filled in address for %s\n", host);
    return RCP_NO_ERROR;
}



RCP_Error rcp_send(rcp_connection *rcp_conn, const void *buf, size_t len){
    return RCP_NO_ERROR;
}


int32_t rcp_close(int32_t fd){
    return close(fd);
}


ssize_t rcp_send_packet(rcp_connection *rcp_conn, Packet *packet){

    //Encryption needs to happen here before packet goes out.

    ssize_t total = 0;
    //This serialization allows for variable length packets
    ssize_t needToSend = sizeof(Packet)+extractDataSize(packet);
    char *buff = malloc(needToSend);
    //Copy over packet header. Pointer to data is invalid on other end.
    memcpy(buff, packet, sizeof(Packet));
    //Now copy over the payload
    memcpy(buff+sizeof(Packet), packet->data, extractDataSize(packet));

    //Send the data as a single UDP packet
    total = sendto(rcp_conn->fd, buff, needToSend, 0,
     (struct sockaddr *)(&(rcp_conn->dest_addr)), sizeof(rcp_sockaddr_in));

    free(buff); //No need for buff anymore
    return total;

}

ssize_t rcp_receive_packet(rcp_connection *rcp_conn, Packet *packet){
    ssize_t total = 0;

    //Decryption needs to happen here and needs to pass to send packets up.

    //Fill in packet struct
    total += recvfrom(rcp_conn->fd, packet, sizeof(Packet), 0, (struct sockaddr *)(&(rcp_conn->dest_addr)), NULL);
    //Make space to hold packet contents
    ssize_t dataSize = extractDataSize(packet);
    packet->data = malloc(dataSize);
    total += recvfrom(rcp_conn->fd, packet->data, dataSize, 0, (struct sockaddr *)(&(rcp_conn->dest_addr)), NULL);
    return total;
}
