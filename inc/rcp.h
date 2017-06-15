#ifndef RCP_H
#define RCP_H

#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h> //for sockaddr_in

typedef struct rcp_sockaddr_in{
    struct sockaddr_in addr;
} rcp_sockaddr_in;

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
 * Closes socket
 * @param  fd File descriptor of socket
 * @return    Same return value as standard socket close
 */
int32_t rcp_close(int32_t fd);

#endif
