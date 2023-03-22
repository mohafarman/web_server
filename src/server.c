#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>		// SOMAXCONN
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>			// close
#include <errno.h>

#define PORT "3490"
#define BUFFER_SIZE 2048
#define ENABLE_DEBUG 1		// Enables debug code if set to 1

void 
sigchld_handler(int signum) {

	(void)signum;
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void 
*get_in_addr(struct sockaddr *sa) {

	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

uint16_t 
get_port(struct sockaddr_storage addr) {
	if (addr.ss_family == AF_INET) {			// IPv4
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		return ntohs(s->sin_port);
	} else if (addr.ss_family == AF_INET6) {	// IPv6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		return ntohs(s->sin6_port);
	} else {
		fprintf(stderr, "Unknown address family\n");
		return 0;
	}
}

int main(int argc, char *argv[]) {

	int verbose = 0;
	int sockfd, newfd;
	int return_value = 0, yes = 1;
	char buffer[BUFFER_SIZE];
	char str_inet[INET_ADDRSTRLEN];
	char *response = "HTTP/1.0 200 OK\r\n"
		"Server: web_server\r\n"
		"Content-type: text/html\r\n\r\n"
		"<html>Hello, World</html>\r\n";
	socklen_t sin_size;
	pid_t pid;
	struct addrinfo hints, *results, *p;
	struct sockaddr_storage their_addr;
	struct sigaction sig_action;

	// Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        }
    }

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;		// IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_flags = AI_PASSIVE;		// use my IP

	if (getaddrinfo(NULL, PORT, &hints, &results) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_value));
	}

	// Loop through all the results and bind to the first we can
	for (p = results; p != NULL; p = p->ai_next) {

		// Attempt to open a TCP network connection throw error
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Server: socket\n");
			continue;
		}

		// Allow reuse of the port if the socket is still "hanging around" in the kernel after
		// previous use
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			perror("Server: setsockopt");
			exit(EXIT_FAILURE);
		}

		// Attempt at binding to the socket
		// "assigning a name to a socket"
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("Server: bind\n");
			continue;
		}

		break;
	}

	freeaddrinfo(results);

	if (p == NULL) {
		fprintf(stderr, "Server: Failed to bind to socket.\n");
		exit(EXIT_FAILURE);
	}

	// SOMAXCONN is a constant defined by the OS
	// set to 128
	if (listen(sockfd, SOMAXCONN) != 0) {
		perror("Server: Failed to listen\n");
	}

	// Function pointer (signal-catching function), 
	// sigchld_handler catches the signal and processes it
	sig_action.sa_handler = sigchld_handler;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sig_action, NULL) != 0) {
		perror("Server: sigaction\n");
		exit(EXIT_FAILURE);
	}

	printf("Server: waiting for connections on port %s...\n", PORT);

	while(1) {

		sin_size = sizeof their_addr;

		newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (newfd == -1) {
			perror("Server: accept\n");
			exit(EXIT_FAILURE);
		}

		inet_ntop(their_addr.ss_family, 
				get_in_addr((struct sockaddr *)&their_addr),
				str_inet, sizeof str_inet);

		printf("Server: got connection from %s:%d\n", str_inet, get_port(their_addr));

		if ((pid = fork()) == -1) {
			perror("Fork failed\n");
		}

		switch(pid) {
			case 0:			// Child process
				close(sockfd);

				if (read(newfd, buffer, 1024) == -1) {
					perror("Server: read\n");
				}

				printf("%s", buffer);

				if (send(newfd, response, strlen(response), 0) == -1) {
					perror("Server: send\n");
				}
				close(newfd);
				exit(EXIT_SUCCESS);

			default:		// Parent process
							// Parent doesn't need this fd
				close(newfd);
		}
	}

	return 0;
}

