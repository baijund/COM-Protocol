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
 * @param  s        RCP_ACTION that occured
 */
static void transitionState(rcp_connection *rcp_conn, RCP_ACTION action);


#if DEBUG
/**
 * Converts RCP_STATE to string representation
 * @param  s RCP_STATE
 * @return   String representing the state
 */
static char *stateToStr(RCP_STATE s);

/**
 * Returns string representation of action
 * @param  action RCP_ACTION
 * @return        string representing action
 */
static char *actionToStr(RCP_ACTION action);

#endif


rcp_connection rcp_initConnection(){
    rcp_connection rcp_conn;
    rcp_conn.fd = -1;
    rcp_conn.seq = 0;
    rcp_conn.ack = 0;
    rcp_conn.state = RCP_UNINIT;
    rcp_conn.clientMode = false;
    rcp_conn.timeouts = rcp_initTimeouts(); //Default values unless user changes.
    return rcp_conn;
}

rcp_timeouts rcp_initTimeouts(){

    rcp_timeouts tos;

    tos.RCP_RCVD_SYN_TO.tv_sec = RCP_RCVD_SYN_TO_SEC;
    tos.RCP_RCVD_SYN_TO.tv_usec = RCP_RCVD_SYN_TO_USEC;

    tos.RCP_SYN_SENT_TO.tv_sec = RCP_SYN_SENT_TO_SEC;
    tos.RCP_SYN_SENT_TO.tv_usec = RCP_SYN_SENT_TO_USEC;

    return tos;
}

void rcp_setTimeouts(rcp_connection *rcp_conn, rcp_timeouts rcp_tos){
    rcp_conn->timeouts = rcp_tos;
}


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
    conn->state = RCP_CLOSED; //Now connection can happen.
    return RCP_NO_ERROR;
}

RCP_Error rcp_connect(rcp_connection *rcp_conn){

    if(rcp_conn->state != RCP_CLOSED){
        DEBUG_PRINT("Can't start connect because connection isn't closed.\n");
        return RCP_NOT_CLOSED; //Can't start connect if not closed
    }

    while(rcp_conn->state != RCP_ESTABLISHED){

        switch(rcp_conn->state){
            case RCP_CLOSED:{
                transitionState(rcp_conn, RCP_IS_GROUND); //Only ground initializes connections
                break;
            }
            case RCP_SYN_SENT:{
                //Wait for synack
                Packet synack; //To hold the synack
                ssize_t rec = rcp_receive_packet(rcp_conn, &synack, rcp_conn->timeouts.RCP_SYN_SENT_TO);
                if(rec<0){
                    DEBUG_PRINT("Some error during receiving synack: %d. Assuming TIMEOUT.\n", errno);
                    transitionState(rcp_conn, RCP_TIMEOUT);
                }else if(isSynAck(&synack)){
                    transitionState(rcp_conn, RCP_RCV_SYNACK);
                } else {
                    transitionState(rcp_conn, RCP_NO_ACTION);
                }
                break;
            }
            default:{
                DEBUG_PRINT("Arrived at invalid state in connect");
                return RCP_INVALID_STATE;
            }
        }
    }

    return RCP_NO_ERROR;
}


RCP_Error rcp_listen(rcp_connection *rcp_conn){
    if(rcp_conn->state != RCP_CLOSED){
        DEBUG_PRINT("Can't start connect because connection isn't closed.\n");
        return RCP_NOT_CLOSED; //Can't start connect if not closed
    }

    while(rcp_conn->state != RCP_ESTABLISHED){
        switch(rcp_conn->state){
            case RCP_CLOSED:{
                transitionState(rcp_conn, RCP_IS_SAT); //Only satellite listens for connections
                break;
            }
            case RCP_LISTEN:{
                //Wait for syn
                Packet syn; //To hold the syn
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 0;
                ssize_t rec = rcp_receive_packet(rcp_conn, &syn, tv);
                if(rec<0){
                    DEBUG_PRINT("Some error during receiving syn: %d. Assuming TIMEOUT.,\n", errno);
                    transitionState(rcp_conn, RCP_TIMEOUT);
                }
                if(isSyn(&syn)){
                    transitionState(rcp_conn, RCP_RCV_SYN);
                } else {
                    transitionState(rcp_conn, RCP_NO_ACTION);
                }
                break;
            }
            case RCP_RCVD_SYN:{
                //Wait for ack
                Packet ack; //To hold the ack
                ssize_t rec = rcp_receive_packet(rcp_conn, &ack, rcp_conn->timeouts.RCP_RCVD_SYN_TO);
                if(rec<0){
                    DEBUG_PRINT("Some error during receiving ack: %d. Assuming TIMEOUT\n", errno);
                    transitionState(rcp_conn, RCP_TIMEOUT);
                }
                if(isAck(&ack)){
                    transitionState(rcp_conn, RCP_RCV_ACK);
                } else {
                    transitionState(rcp_conn, RCP_NO_ACTION);
                }
                break;
            }
            default:{
                DEBUG_PRINT("Arrived at invalid state in listen\n");
                return RCP_INVALID_STATE;
            }
        }
    }
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

    //Serialize the packet
    void *buff = serializePacket(packet);

    //Send the data as a single UDP packet
    total = sendto(rcp_conn->fd, buff, PACKET_SERIAL_SIZE(packet), 0,
     (struct sockaddr *)(&(rcp_conn->dest_addr)), sizeof(rcp_sockaddr_in));

    free(buff); //No need for buff anymore
    return total;

}

ssize_t rcp_receive_packet(rcp_connection *rcp_conn, Packet *packet, struct timeval tv){

    //Decryption needs to happen here and needs to pass to send packets up.



    //Configure the socket for timeout.
    setsockopt(rcp_conn->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

    struct sockaddr_in remaddr;     /* remote address */
    socklen_t addrlen = sizeof(remaddr);            /* length of addresses */

    // //Fill in packet struct
    char buff[PACKET_HEAD_SIZE];
    ssize_t recret = 0;
    //Only peek so that the datagram is not discarded.
    recret = recvfrom(rcp_conn->fd, buff, PACKET_HEAD_SIZE, MSG_PEEK, (struct sockaddr *)&remaddr, &addrlen);

    //Unset the timeout after the peek
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(rcp_conn->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));



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

static void sendSyn(rcp_connection *rcp_conn){
    //Send syn
    Packet *syn = createPacket(true, false, rcp_conn->seq, 0, NULL);
    rcp_send_packet(rcp_conn, syn); //Send the packet over
    destroyPacket(syn);
}

static void sendSynAck(rcp_connection *rcp_conn){
    //Received syn, send synack
    Packet *synack = createPacket(true, true, rcp_conn->seq, 0, NULL);
    rcp_send_packet(rcp_conn, synack); //Send the packet over
    destroyPacket(synack);
}

static void sendAck(rcp_connection *rcp_conn){
    //Received ack, send ack
    Packet *ack = createPacket(false, true, rcp_conn->seq, 0, NULL);
    rcp_send_packet(rcp_conn, ack); //Send the packet over
    destroyPacket(ack);
}

#define INVALID_ACTION(rcp_conn, action) DEBUG_PRINT("%s is an invalid action at %s\n", actionToStr(action), stateToStr(rcp_conn->state));

static void transitionState(rcp_connection *rcp_conn, RCP_ACTION action){
    DEBUG_PRINT("Currently at %s and received input %s\n", stateToStr(rcp_conn->state), actionToStr(action));

    RCP_ACTION executedAction = RCP_NO_ACTION;
    switch(rcp_conn->state){
        case RCP_UNINIT:{
            break; //Can't do anything if state is uninitialized
        }
        case RCP_CLOSED:{
            switch(action){
                case RCP_IS_GROUND:{
                    sendSyn(rcp_conn);
                    rcp_conn->state = RCP_SYN_SENT;
                    executedAction = RCP_SEND_SYN;
                    break;
                }
                case RCP_IS_SAT:{
                    //Go straight to listen
                    rcp_conn->state = RCP_LISTEN;
                    break;
                }
                default:{
                    INVALID_ACTION(rcp_conn, action);
                    break;
                }
            }
            break;
        }
        case RCP_SYN_SENT:{
            switch(action){
                case RCP_RCV_SYNACK:{
                    sendAck(rcp_conn);
                    executedAction = RCP_SEND_ACK;
                    rcp_conn->state = RCP_ESTABLISHED;
                    break;
                }
                case RCP_TIMEOUT:{
                    sendSyn(rcp_conn);
                    executedAction = RCP_SEND_SYN;
                    break;
                }
                default:{
                    INVALID_ACTION(rcp_conn, action);
                    break;
                }
            }
            break;
        }
        //TODO
        case RCP_LISTEN:{
            switch(action){
                case RCP_RCV_SYN:{
                    sendSynAck(rcp_conn);
                    rcp_conn->state = RCP_RCVD_SYN;
                    executedAction = RCP_SEND_SYNACK;
                    break;
                }
                default:{
                    INVALID_ACTION(rcp_conn, action);
                    break;
                }
            }
            break;
        }
        //TODO
        case RCP_RCVD_SYN:{
            switch(action){
                case RCP_TIMEOUT:{
                    sendSynAck(rcp_conn);
                    executedAction = RCP_SEND_SYNACK;
                    break;
                }
                case RCP_RCV_ACK:{
                    rcp_conn->state = RCP_ESTABLISHED;
                    executedAction = RCP_NO_ACTION;
                    break;
                }
                default:{
                    INVALID_ACTION(rcp_conn, action);
                    break;
                }
            }
            break;
        }
        //TODO
        case RCP_ESTABLISHED:{
            break;
        }
        default:{
            INVALID_ACTION(rcp_conn, action);
            break;
        }
    }

    DEBUG_PRINT("Transitioned to %s and executed %s\n", stateToStr(rcp_conn->state), actionToStr(executedAction));

}

#if DEBUG

static char *stateToStr(RCP_STATE s){
    switch(s){
        case RCP_CLOSED:{
            return "RCP_CLOSED";
        }
        case RCP_SYN_SENT:{
            return "RCP_SYN_SENT";
        }
        case RCP_LISTEN:{
            return "RCP_LISTEN";
        }
        case RCP_RCVD_SYN:{
            return "RCP_RCVD_SYN";
        }
        case RCP_ESTABLISHED:{
            return "RCP_ESTABLISHED";
        }
        default:{
            return "NOT A VALID STATE!";
        }
    }
}

static char *actionToStr(RCP_ACTION action){
    switch(action){
        case RCP_NO_ACTION:{
            return "RCP_NO_ACTION";
        }
        case RCP_IS_GROUND:{
            return "RCP_IS_GROUND";
        }
        case RCP_IS_SAT:{
            return "RCP_IS_SAT";
        }
        case RCP_RCV_SYN:{
            return "RCP_RCVD_SYN";
        }
        case RCP_RCV_SYNACK:{
            return "RCP_RCV_SYNACK";
        }
        case RCP_RCV_ACK:{
            return "RCP_RCV_ACK";
        }
        case RCP_SEND_SYN:{
            return "RCP_SEND_SYN";
        }
        case RCP_SEND_ACK:{
            return "RCP_SEND_ACK";
        }
        case RCP_SEND_SYNACK:{
            return "RCP_SEND_SYNACK";
        }
        case RCP_TIMEOUT:{
            return "RCP_TIMEOUT";
        }
        default:{
            return "INVALID_ACTION";
        }
    }
}

#endif
