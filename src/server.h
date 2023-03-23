#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>		// SOMAXCONN
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "3490"
#define BUFFER_SIZE 2048

void 
sigchld_handler(int signum);

void 
*get_in_addr(struct sockaddr *sa);

uint16_t 
get_port(struct sockaddr_storage addr);

#endif /* SERVER_H */
