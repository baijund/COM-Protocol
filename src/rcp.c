#include <string.h>
#include <unistd.h> //For close()
#include <netdb.h> //For host resolution
#include "rcp_config.h"
#include "rcp.h"

#include <errno.h>
extern int32_t errno;

/**
 * Transitions to state s.
 * @param  rcp_conn Connection information
 * @param  s        RCP_STATE to transiton to
 */
// static void transitionState(rcp_connection *rcp_conn, RCP_STATE s1, RCP_STATE s2);

int32_t rcp_socket(rcp_connection *rcp_conn){
    DEBUG_PRINT("Creating UDP socket.\n");
    rcp_conn->fd = (int32_t) socket(AF_INET, SOCK_DGRAM, 0);
    rcp_conn->state = RCP_CLOSED;
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

RCP_Error rcp_connect(rcp_connection *rcp_conn){

    while(1){
        //Send syn
        //First create syn packet
        Packet *syn = createPacket(true, false, rcp_conn->seq, 0, NULL);
        rcp_send_packet(rcp_conn, syn); //Send the packet over
        rcp_conn->state = RCP_SYN_SENT; //Transition to SYN_SENT state

        //Wait for synack
        Packet synack; //To hold the synack
        ssize_t rec = rcp_receive_packet(rcp_conn, &synack);
        if(rec<0){
            int32_t err = errno;
            DEBUG_PRINT("Some error during receiving synack: %d\n", err);
        }

        //Send ack
        if(isSynAck(&synack)){
            rcp_conn->state = RCP_ESTABLISHED;
        }
    }
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

    //Serialize the packet
    void *buff = serializePacket(packet);

    //Send the data as a single UDP packet
    total = sendto(rcp_conn->fd, buff, PACKET_SERIAL_SIZE(packet), 0,
     (struct sockaddr *)(&(rcp_conn->dest_addr)), sizeof(rcp_sockaddr_in));

    free(buff); //No need for buff anymore
    return total;

}

ssize_t rcp_receive_packet(rcp_connection *rcp_conn, Packet *packet){

    //Decryption needs to happen here and needs to pass to send packets up.

    struct sockaddr_in remaddr;     /* remote address */
    socklen_t addrlen = sizeof(remaddr);            /* length of addresses */

    // //Fill in packet struct
    char buff[PACKET_HEAD_SIZE];
    ssize_t recret = 0;
    //Only peek so that the datagram is not discarded.
    recret = recvfrom(rcp_conn->fd, buff, PACKET_HEAD_SIZE, MSG_PEEK, (struct sockaddr *)&remaddr, &addrlen);
    if(recret<0){
        // int err = errno;
        // DEBUG_PRINT("errno is %d\n",err);
        // DEBUG_PRINT("recvfrom returned %ld\n", recret);
        return recret;
    }

    uint32_t *seqpos = (uint32_t *)(buff+2);
    packet->dataSize = ntohl(seqpos[1]); //This is the amount of data that is in the packet.

    //Create a buffer long enough to hold the whole packet
    char *serialPacket = malloc(PACKET_SERIAL_SIZE(packet));

    //Receive no peek
    recret = recvfrom(rcp_conn->fd, serialPacket, PACKET_SERIAL_SIZE(packet), 0, (struct sockaddr *)&remaddr, &addrlen);
    if(recret<0){
        int err = errno;
        DEBUG_PRINT("errno is %d\n",err);
        DEBUG_PRINT("recvfrom returned %ld\n", recret);
        return recret;
    }
    deserializePacket(serialPacket, packet);
    free(serialPacket); //No need for serialized packet when packet is created.

    return recret;
}

//
// static void transitionState(rcp_connection *rcp_conn, RCP_STATE s){
//
//     rcp_conn->state = s;
// }
//
// static char *stateToStr(RCP_STATE s){
//     switch(s){
//         case RCP_CLOSED:{
//             return "RCP_CLOSED";
//         }
//         case RCP_SYN_SENT:{
//             return "RCP_SYN_SENT";
//         }
//         case RCP_LISTEN:{
//             return "RCP_LISTEN";
//         }
//         case RCP_RCVD_SYN:{
//             return "RCP_RCVD_SYN";
//         }
//         case RCP_ESTABLISHED:{
//             return "RCP_ESTABLISHED";
//         }
//     }
// }
