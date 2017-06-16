#ifndef RCP_H
#define RCP_H

#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h> //for sockaddr_in

typedef struct {
    struct sockaddr_in addr;
} rcp_sockaddr_in;


typedef enum {
    SEND_NO_ERROR,
    SEND_NO_CONNECTION,
    SEND_GENERIC_ERROR
} RCP_Send_Error;

typedef struct {
    int32_t fd; //The file descriptor of the associated socket.
    int32_t seq; //The outbound sequence number.
    int32_t ack; //The outbound ack number.
} rcp_connection;

/**
 * Creates a UDP socket with same return values as socket in standard library
 * @return Socket file descriptor
 */
int32_t rcp_socket();


/**
 * Binds socket to a port
 * @param  fd       Socket file descriptor
 * @param  rcp_sock Pointer to structure to hold filled out information
 * @param  port     Port that needs to be bound to. 0 for any port.
 * @return          Same return value as standart bind.
 */
int32_t rcp_bind(int32_t fd, rcp_sockaddr_in *rcp_sock, uint16_t port);

/**
 * SImilar to TCP send. Adds data to the send buffer for transmission.
 * @param  rcp_conn The rcp_connection struct associated with the connection
 * @param  buf      The message to be transmitted.
 * @param  len      The length of the message.
 * @return          Send error if one occured.
 */
RCP_Send_Error rcp_send(rcp_connection *rcp_conn, const void *buf, size_t len);

/**
 * Closes socket
 * @param  fd File descriptor of socket
 * @return    Same return value as standard socket close
 */
int32_t rcp_close(int32_t fd);



#endif
