#include <string.h>
#include <unistd.h> //For close()
#include <netdb.h> //For host resolution
#include <sys/time.h> //For sleeping and timeval arithmetic
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


static void *rcp_establishedDaemon(void *conn);


/**
 * Returns min of two uint32_t
 * @param  u1 [description]
 * @param  u2 [description]
 * @return    [description]
 */
static uint32_t rcp_uint_min(uint32_t u1, uint32_t u2){
    return u1<u2?u1:u2;
}

/**
 * returns max of u1 and u2
 * @param  u1 [description]
 * @param  u2 [description]
 * @return    [description]
 */
static uint32_t rcp_uint_max(uint32_t u1, uint32_t u2){
    return u1>u2?u1:u2;
}

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
    rcp_conn.established = false;
    rcp_conn.timeouts = rcp_initTimeouts(); //Default value unless programatically changed.
    rcp_conn.serverRetries = RCP_SERVER_RETRIES; //Default value unless programatically changed
    rcp_conn.slidingWindowLen = RCP_SLIDING_WINDOW_LEN; //Default value unless programatically changed
    rcp_conn.daemonSpawned = false;
    rcp_conn.maxPacketDataSize = RCP_MAX_PACKET_DATA_SIZE;

    rcp_set_switch_time(&rcp_conn, RCP_SWITCH_US, RCP_SWITCH_SEC);

    if(pthread_mutex_init(&(rcp_conn.sendLock), NULL)){
        DEBUG_PRINT("Failed to initialize send mutex.\n");
    }
    if(pthread_mutex_init(&(rcp_conn.receiveLock), NULL)){
        DEBUG_PRINT("Failed to initialize receive mutex.\n");
    }
    return rcp_conn;
}

void rcp_set_switch_time(rcp_connection *rcp_conn, uint32_t us, uint32_t sec){
    rcp_conn->switchTime.tv_usec = us;
    rcp_conn->switchTime.tv_sec = sec;
}

rcp_timeouts rcp_initTimeouts(){

    rcp_timeouts tos;

    tos.RCP_RCVD_SYN_TO.tv_sec = RCP_RCVD_SYN_TO_SEC;
    tos.RCP_RCVD_SYN_TO.tv_usec = RCP_RCVD_SYN_TO_USEC;

    tos.RCP_SYN_SENT_TO.tv_sec = RCP_SYN_SENT_TO_SEC;
    tos.RCP_SYN_SENT_TO.tv_usec = RCP_SYN_SENT_TO_USEC;

    tos.RCP_STATE_SYN_SENT_TO.tv_sec = RCP_STATE_SYN_SENT_TO_SEC;
    tos.RCP_STATE_SYN_SENT_TO.tv_usec = RCP_STATE_SYN_SENT_TO_USEC;


    tos.RCP_STATE_RCVD_SYN_TO.tv_sec = RCP_STATE_RCVD_SYN_TO_SEC;
    tos.RCP_STATE_RCVD_SYN_TO.tv_usec = RCP_STATE_RCVD_SYN_TO_USEC;


    tos.RCP_ESTABLISHED_SERVER_TO.tv_sec = RCP_ESTABLISHED_SERVER_TO_SEC;
    tos.RCP_ESTABLISHED_SERVER_TO.tv_usec = RCP_ESTABLISHED_SERVER_TO_USEC;

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

    while(!rcp_conn->established){

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
                    break;
                }else if(isSynAck(&synack)){
                    transitionState(rcp_conn, RCP_RCV_SYNACK);
                } else {
                    transitionState(rcp_conn, RCP_NO_ACTION);
                }
                destroyPacketData(&synack);
                break;
            }
            default:{
                DEBUG_PRINT("Arrived at invalid state in connect");
                return RCP_INVALID_STATE;
            }
        }
    }

    //Spawn a daemon to handle the established state
    pthread_create(&rcp_conn->tid, NULL, rcp_establishedDaemon, rcp_conn);
    rcp_conn->daemonSpawned = true;


    return RCP_NO_ERROR;
}

RCP_Error rcp_listen(rcp_connection *rcp_conn){
    if(rcp_conn->state != RCP_CLOSED){
        DEBUG_PRINT("Can't start connect because connection isn't closed.\n");
        return RCP_NOT_CLOSED; //Can't start connect if not closed
    }

    while(!rcp_conn->established){
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
                    break;
                }
                if(isSyn(&syn)){
                    transitionState(rcp_conn, RCP_RCV_SYN);
                } else {
                    transitionState(rcp_conn, RCP_NO_ACTION);
                }
                destroyPacketData(&syn);
                break;
            }
            case RCP_RCVD_SYN:{
                //Wait for ack
                Packet ack; //To hold the ack
                ssize_t rec = rcp_receive_packet(rcp_conn, &ack, rcp_conn->timeouts.RCP_RCVD_SYN_TO);
                if(rec<0){
                    DEBUG_PRINT("Some error during receiving ack: %d. Assuming TIMEOUT\n", errno);
                    transitionState(rcp_conn, RCP_TIMEOUT);
                    break;
                }
                if(isAck(&ack)){
                    transitionState(rcp_conn, RCP_RCV_ACK);
                } else {
                    transitionState(rcp_conn, RCP_NO_ACTION);
                }
                destroyPacketData(&ack);
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

static void *rcp_establishedDaemon(void *conn){
    rcp_connection *rcp_conn = (rcp_connection *)conn;
    DEBUG_PRINT("ESTABLISHED daemon spawned!\n");

    rcp_conn->ack = 1; //Expecting a packet with the sequence number of 1
    rcp_conn->seq = 1; //Will send packets starting with the sequence number of 1;
    rcp_conn->unackedPacketCount = 0; //No packets are waiting on an ack currently

    while(rcp_conn->established){
        if(rcp_conn->state == RCP_ESTABLISHED_CLIENT){
            //This machine is in client mode
            if(get_queue_size(rcp_conn->sendBuffer)){
                //There are packets to send
                pthread_mutex_lock(&rcp_conn->sendLock);
                uint32_t ableToSend = rcp_uint_min(rcp_conn->slidingWindowLen, get_queue_size(rcp_conn->sendBuffer));
                QueueEntry *entry = rcp_conn->sendBuffer->head; //Start at head of send buffer
                for(uint32_t i = 0; i < ableToSend; i++){
                    if(entry == NULL){
                        DEBUG_PRINT("Entry is NULL!\n");
                    }
                    Packet *pack = entry->data;
                    pack->seq = rcp_conn->seq+i+1; //+1 because rcp_conn->seq represents the outbound packets that have been ackd
                    rcp_send_packet(rcp_conn, pack);
                    entry = entry->next;
                }
                pthread_mutex_unlock(&rcp_conn->sendLock);
                DEBUG_PRINT("Sent %d packets\n", ableToSend);
                Packet pack;
                //Now wait for an ack
                int32_t rec = rcp_receive_packet(rcp_conn, &pack, rcp_conn->timeouts.RCP_ESTABLISHED_SERVER_TO); //Wait at least as long as it takes for the server to acknowledge
                if(rec<0){
                    continue; //Assume a timeout
                }
                if(isAck(&pack) && extractSeq(&pack)>=rcp_conn->seq){
                    rcp_conn->seq = rcp_uint_max(extractSeq(&pack),rcp_conn->seq); //The sequence number of the outbound packets starts from the highest acknowledged packet
                }
                pthread_mutex_lock(&(rcp_conn->sendLock));
                while(rcp_conn->sendBuffer->head && rcp_conn->seq>=extractSeq((Packet *)rcp_conn->sendBuffer->head->data)){
                    Packet *pack = queue_pop_tail(rcp_conn->sendBuffer);
                    destroyPacket(pack);
                }
                pthread_mutex_unlock(&(rcp_conn->sendLock));
                destroyPacketData(&pack);
            } else {
                transitionState(rcp_conn, RCP_NOTHING_TO_SEND);
            }

        } else {
            //This machine is in server mode
            uint32_t serverRetries = 0;
            bool serverTimeout = false;

            uint32_t numReceivedPackets = 0;
            while((numReceivedPackets<rcp_conn->slidingWindowLen)){
                Packet *pack = createPacket(0, 0, 0, 0, NULL);
                int rec = rcp_receive_packet(rcp_conn, pack, rcp_conn->timeouts.RCP_ESTABLISHED_SERVER_TO);

                if(rec<0){
                    //Assume a timeout
                    DEBUG_PRINT("RCV timeout occured.\n");
                    destroyPacket(pack);
                    if(serverRetries++ >= rcp_conn->serverRetries){
                        serverTimeout = true; //Timeout has happened. Now the machine should ack last packet and decide whether to switch to client mode
                        DEBUG_PRINT("Server timeout\n");
                        break;
                    }
                    DEBUG_PRINT("Server retries is %d\n", serverRetries);
                    continue;
                }
                if(extractSeq(pack) == (rcp_conn->ack+1)){
                    numReceivedPackets++;
                    pthread_mutex_lock(&rcp_conn->receiveLock);
                    queue_push_head(rcp_conn->receiveBuffer, pack); //Now the packet memory is owned by the queue
                    rcp_conn->bytesInRecBuff+=extractDataSize(pack);
                    rcp_conn->ack++;    //Increase expected ack #
                    pthread_mutex_unlock(&rcp_conn->receiveLock);
                }

            }
            if(serverTimeout && (numReceivedPackets<rcp_conn->slidingWindowLen)){
                transitionState(rcp_conn, RCP_SERV_TO_AND_LESS_PACKS);
            }
            if(get_queue_size(rcp_conn->sendBuffer)){
                transitionState(rcp_conn, RCP_SOMETHING_TO_SEND);
            }
        }
    }
    DEBUG_PRINT("ESTABLISHED daemon is dying.\n");
    return NULL;
}


//TODO
RCP_Error rcp_send(rcp_connection *rcp_conn, uint8_t const *buf, uint32_t len){
    //First packetize the buffer
    while(len != 0){
        uint32_t amountUsed = rcp_uint_min(rcp_conn->maxPacketDataSize, len);
        Packet *pack = createPacket(false, false, 0, amountUsed, buf);
        pthread_mutex_lock(&(rcp_conn->sendLock));
        queue_push_tail(rcp_conn->sendBuffer, pack);
        pthread_mutex_unlock(&(rcp_conn->sendLock));
        len-=amountUsed;
    }
    DEBUG_PRINT("%d bytes added to send buffer.\n", len);
    return RCP_NO_ERROR;
}

//TODO
RCP_Error rcp_receive(rcp_connection *rcp_conn, uint8_t *buf, uint32_t len, struct timeval const to){
    return RCP_NO_ERROR;
}


int32_t rcp_close(rcp_connection *rcp_conn){
    rcp_conn->established = false;
    if(rcp_conn->daemonSpawned){
        pthread_join(rcp_conn->tid, NULL); //Wait for daemon to die.
    }
    rcp_conn->state = RCP_UNINIT;
    return close(rcp_conn->fd);
}

/**
 * Returns the amount of time remaining until next send is allowed. This is to account for TX to RX and RX to TX switching delays.
 * @param  rcp_conn Used to determine when last receive occured. Also used to
 * @return          [description]
 */
static struct timeval timeRemainingToSend(rcp_connection *rcp_conn){
    struct timeval tv_elapsed; //Will hold the amount of time elapsed since the last received packet
    struct timeval lastv = rcp_conn->lastReceivedTime;
    struct timeval currtv;
    gettimeofday(&currtv, NULL);
    timersub(&currtv, &lastv, &tv_elapsed);

    struct timeval tv_rem; //Will hold the amount of time remaining.

    timersub(&rcp_conn->switchTime, &tv_elapsed, &tv_rem);

    return tv_rem;
}

ssize_t rcp_send_packet(rcp_connection *rcp_conn, Packet *packet){

    //Encryption needs to happen here before packet goes out.

    ssize_t total = 0;

    //Serialize the packet
    void *buff = serializePacket(packet);

    //Sleep until a packet can be sent
    struct timeval tv = timeRemainingToSend(rcp_conn);
    if(tv.tv_sec>=0){
        sleep(tv.tv_sec);
        usleep(tv.tv_usec);
    }

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
    } else {
        gettimeofday(&rcp_conn->lastReceivedTime, NULL);
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
    Packet *ack = createPacket(false, true, rcp_conn->ack, 0, NULL);
    rcp_send_packet(rcp_conn, ack); //Send the packet over
    destroyPacket(ack);
}

#define INVALID_ACTION(rcp_conn, action) DEBUG_PRINT("%s is an invalid action at %s\n", actionToStr(action), stateToStr(rcp_conn->state));

/**
 * Returns true if there is still time remaining to wait in the state. Or else false.
 * @param  rcp_conn Struct used to check when state was entered.
 * @param  timeout  Time allowed for state
 * @return
 */
static uint8_t stateWithinTime(rcp_connection *rcp_conn, struct timeval timeout){
    struct timeval start = rcp_conn->stateStartTime;
    struct timeval current;
    gettimeofday(&current, NULL);

    struct timeval elapsed;
    timersub(&current, &start, &elapsed);
    // DEBUG_PRINT("%ld seconds elapsed. %ld usec elapsed.\n", (int64_t)elapsed.tv_sec, elapsed.tv_usec);

    uint8_t ret = timercmp(&elapsed, &timeout, <);

    if(!ret){
        DEBUG_PRINT("State Timeout: Time elapsed is greater than %ld sec and %ld usec\n", timeout.tv_sec, timeout.tv_usec);
    } else {
        DEBUG_PRINT("No State Timeout: Time elapsed is less than %ld sec and %ld usec\n", timeout.tv_sec, timeout.tv_usec);
    }
    return ret;
}

static void adjustState(rcp_connection *rcp_conn, RCP_STATE state){
    rcp_conn->state = state;
    gettimeofday(&(rcp_conn->stateStartTime), NULL); //TODO address the assumption that this doesnt error.
    DEBUG_PRINT("Entered state %s at %ld seconds, and %ld microseconds.\n", stateToStr(rcp_conn->state), rcp_conn->stateStartTime.tv_sec, rcp_conn->stateStartTime.tv_usec);
}

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
                    adjustState(rcp_conn, RCP_SYN_SENT);
                    // rcp_conn->state = RCP_SYN_SENT;
                    executedAction = RCP_SEND_SYN;
                    break;
                }
                case RCP_IS_SAT:{
                    //Go straight to listen
                    adjustState(rcp_conn, RCP_LISTEN);
                    // rcp_conn->state = RCP_LISTEN;
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
                    adjustState(rcp_conn, RCP_ESTABLISHED_SERVER);
                    // rcp_conn->state = RCP_ESTABLISHED_SERVER;
                    rcp_conn->established = true;
                    break;
                }
                case RCP_TIMEOUT:{
                    uint8_t noTimeout = stateWithinTime(rcp_conn, rcp_conn->timeouts.RCP_STATE_SYN_SENT_TO);
                    if(!noTimeout){
                        //A state timeout occured during connection phase. Go to closed.
                        adjustState(rcp_conn, RCP_CLOSED);
                    } else{
                        sendSyn(rcp_conn);
                        executedAction = RCP_SEND_SYN;
                    }
                    break;
                }
                default:{
                    INVALID_ACTION(rcp_conn, action);
                    sendSyn(rcp_conn);
                    executedAction = RCP_SEND_SYN;
                    break;
                }
            }
            break;
        }
        case RCP_LISTEN:{
            switch(action){
                case RCP_RCV_SYN:{
                    sendSynAck(rcp_conn);
                    // rcp_conn->state = RCP_RCVD_SYN;
                    adjustState(rcp_conn, RCP_RCVD_SYN);
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
        case RCP_RCVD_SYN:{
            switch(action){
                case RCP_TIMEOUT:{
                    uint8_t timeout = !stateWithinTime(rcp_conn, rcp_conn->timeouts.RCP_STATE_RCVD_SYN_TO);
                    if(timeout){
                        //A state timeout occured during connection phase. Go to closed.
                        adjustState(rcp_conn, RCP_CLOSED);
                    } else {
                        sendSynAck(rcp_conn);
                        executedAction = RCP_SEND_SYNACK;
                    }
                    break;
                }
                case RCP_RCV_ACK:{
                    adjustState(rcp_conn, RCP_ESTABLISHED_SERVER);
                    rcp_conn->established = true;
                    executedAction = RCP_NO_ACTION;
                    break;
                }
                default:{
                    INVALID_ACTION(rcp_conn, action);
                    sendSynAck(rcp_conn);
                    executedAction = RCP_SEND_SYNACK;
                    break;
                }
            }
            break;
        }
        //TODO
        case RCP_ESTABLISHED_CLIENT:{
            switch (action) {
                case RCP_NOTHING_TO_SEND:{
                    executedAction = RCP_NO_ACTION;
                    adjustState(rcp_conn, RCP_ESTABLISHED_SERVER);
                    // rcp_conn->state = RCP_ESTABLISHED_SERVER;
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
        case RCP_ESTABLISHED_SERVER:{
            switch (action) {
                case RCP_RCV_N_PACKS:
                case RCP_SERV_TO_AND_LESS_PACKS:{
                    sendAck(rcp_conn);
                    //Send ack and transition to server mode for both actions
                    adjustState(rcp_conn, RCP_ESTABLISHED_SERVER);
                    // rcp_conn->state = RCP_ESTABLISHED_SERVER;
                    executedAction = RCP_SEND_ACK;
                    break;
                }
                case RCP_SOMETHING_TO_SEND:{
                    adjustState(rcp_conn, RCP_ESTABLISHED_CLIENT);
                    // rcp_conn->state = RCP_ESTABLISHED_CLIENT;
                    executedAction = RCP_NO_ACTION;
                }
                default:{
                    INVALID_ACTION(rcp_conn, action);
                }
            }
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
        case RCP_ESTABLISHED_CLIENT:{
            return "RCP_ESTABLISHED_CLIENT";
        }
        case RCP_ESTABLISHED_SERVER:{
            return "RCP_ESTABLISHED_SERVER";
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
        case RCP_SERV_TO_AND_LESS_PACKS:{
            return "RCP_SERV_TO_AND_LESS_PACKS";
        }
        case RCP_RCV_N_PACKS:{
            return "RCP_RCV_N_PACKS";
        }
        case RCP_NOTHING_TO_SEND:{
            return "RCP_NOTHING_TO_SEND";
        }
        case RCP_SOMETHING_TO_SEND:{
            return "RCP_SOMETHING_TO_SEND";
        }
        default:{
            return "INVALID_ACTION";
        }
    }
}

#endif
