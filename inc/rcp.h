#ifndef RCP_H
#define RCP_H

#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h> //for sockaddr_in

typedef struct sockaddr_in rcp_sockaddr_in;


typedef enum {
    RCP_NO_ERROR,
    RCP_NO_CONNECTION,
    RCP_HOSTNAME_FAIL, //Failed to get host information
    RCP_GENERIC_ERROR
} RCP_Error;

typedef enum {
    RCP_CLOSED,
    RCP_SYN_SENT,
    RCP_LISTEN,
    RCP_RCVD_SYN,
    RCP_ESTABLISHED
} RCP_STATE; //Corresponds to state diagram states

typedef enum {
    RCP_NO_ACTION,
    RCP_IS_GROUND,
    RCP_IS_SAT,
    RCP_RCV_SYN,
    RCP_RCV_SYNACK,
    RCP_RCV_ACK
} RCP_ACTION; //Corresponds to actions in state diagram

/**
 * User should not have to worry about contents of this struct unless to debug.
 */
typedef struct {
    int32_t fd; //The file descriptor of the associated socket.
    int32_t seq; //The outbound sequence number.
    int32_t ack; //The outbound ack number.
    rcp_sockaddr_in dest_addr; //Information of other end
    rcp_sockaddr_in this_addr; //Information of this end
    RCP_STATE state; //The state of the connection.
    bool clientMode; //Determines whether the machine is in client or server mode
} rcp_connection;

/**
 * Creates a UDP socket with same return values as socket in standard library
 * @param rcp_con An rcp_conection struct to fill in connection information.
 * @return Socket file descriptor
 */
 int32_t rcp_socket(rcp_connection *rcp_conn);


/**
 * Binds socket to a port. rcp_conn field is filled out only when successful bind
 * @param  rcp_conn rcp_connection struct to fill in connection information
 * @param  fd       Socket file descriptor
 * @param  rcp_sock Pointer to structure to hold filled out information
 * @param  port     Port that needs to be bound to. 0 for any port.
 * @return          Same return value as standart bind.
 */
int32_t rcp_bind(rcp_connection *rcp_conn, int32_t fd, rcp_sockaddr_in *rcp_sock, uint16_t port);


/**
 * Sets up other endpoint information in connection struct
 * @param  conn rcp_connection struct
 * @param  host other endpoint in dotted ip notation
 * @param  port other endpoint port
 * @return      an RCP_Error depending on error
 */
RCP_Error setupDest(rcp_connection *conn, const char *host, uint16_t port);

/**
 * Establishes connection with endpoint specified in rcp_conn
 * @param  rcp_conn Contains connection information
 * @return          Type of error if any
 */
RCP_Error rcp_connect(rcp_connection *rcp_conn);

/**
 * SImilar to TCP send. Adds data to the send buffer for transmission.
 * @param  rcp_conn The rcp_connection struct associated with the connection
 * @param  buf      The message to be transmitted.
 * @param  len      The length of the message.
 * @return          Send error if one occured.
 */
RCP_Error rcp_send(rcp_connection *rcp_conn, const void *buf, size_t len);

/**
 * Closes socket
 * @param  fd File descriptor of socket
 * @return    Same return value as standard socket close
 */
int32_t rcp_close(int32_t fd);


//These are not static for debugging purposes. User has no need of these functions.
#include "rcp_packet.h"

typedef enum {
    PKT_NO_ERROR,
    PKT_SEND_PACKET_ERROR,
    PKT_RECEIVE_PACKET_ERROR
} Packet_Error;

/**
 * A sendto call to a UDP socket with preset parameters.
 * Serializes packet data and sends in same call with data adjacent to Packet struct.
 * @param  rcp_conn rcp_connection struct containing information to update connection
 * @param  packet   packet to be sent
 * @return          Same return value as sendto()
 */
ssize_t rcp_send_packet(rcp_connection *rcp_conn, Packet *packet);

/**
 * Receive a RCP packet from socket.
 * Unserializes data into single RCP packet from single UDP packet.
 * @param  rcp_conn rcp_connection struct
 * @param  packet   Packet struct to be populated
 * @return          number of bytes received (including packet struct).
 */
ssize_t rcp_receive_packet(rcp_connection *rcp_conn, Packet *packet);

#endif
