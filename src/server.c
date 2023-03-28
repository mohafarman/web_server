#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>			// close
#include <errno.h>

#include "../include/server.h"
#include "../include/utils.h"

#define PROGRAM_NAME "web_server"
#define ENABLE_DEBUG 1		// Enables debug code if set to 1

int main(int argc, char *argv[]) {

	int sockfd, newfd;
	int return_value = 0, yes = 1;
	char client_buffer_request[BUFFER_SIZE];
	char str_inet[INET_ADDRSTRLEN];
	char *response = malloc(BUFFER_SIZE * sizeof(char));
	char *header = "HTTP/1.0 200 OK\r\n"
		"Server: web_server\r\n"
		"Content-type: text/html\r\n\r\n";
	strcat(response, header);
	socklen_t sin_size;
	pid_t pid;
	struct addrinfo hints, *results, *p;
	struct sockaddr_storage their_addr;
	struct sigaction sig_action;

	if (argc < 2) {
		printf("Usage: %s [PORT]", PROGRAM_NAME);
		exit(EXIT_FAILURE);
	}

	char * port = argv[1];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;		// IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_flags = AI_PASSIVE;		// use my IP

	if (getaddrinfo(NULL, port, &hints, &results) != 0) {
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

	printf("Server: waiting for connections on port %s...\n", port);

	while(1) {

		sin_size = sizeof their_addr;

		// Program blocks on accept until a connection has been made
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
				struct http_request *http_request = malloc(sizeof (struct http_request));
				struct http_response *http_response = malloc(sizeof (struct http_response));

				// Read in the client request
				if (read(newfd, client_buffer_request, BUFFER_SIZE) == -1) {
					perror("Server: read\n");
				}

				printf("========\nClient request:\n%s\n========\n", client_buffer_request);

				// Respond accordingly
				
				// Handle client HTTP request
				// GET -> Serve
				handle_client_http_request(client_buffer_request, http_request, http_response);

				char file_buffer[BUFFER_SIZE];
				FILE *index_html = fopen("html/index.html", "rb");
				if (index_html == NULL) {
					fprintf(stderr, "Error opening index.html file\n");
				}

				// Get size of file
				fseek(index_html, 0, SEEK_END);
				long file_size = ftell(index_html);
				fseek(index_html, 0, SEEK_SET);

				// Read file into buffer
				size_t bytes_read = fread(file_buffer, sizeof(char), file_size, index_html);
				//fread(file_buffer, sizeof(char), file_size, index_html);

				// set content length determining bytes read into memory from file 
				// (same as file_size)
				http_response->content_length = (int) bytes_read;

				// Construct the HTTP header
				sprintf(http_response->content, 
						"%s %d %s\r\nServer: %s\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n",
						http_request->protocol_version,
						http_response->status_code,
						http_response->phrase,
						PROGRAM_NAME,
						http_response->content_type,
						http_response->content_length);

				// Append the content (file content) to the response which includes the HTTP header
				strcat(http_response->content, file_buffer);

				if (send(newfd, http_response->content, strlen(http_response->content), 0) == -1) {
					perror("Server: send\n");
				}

				fclose(index_html);
				close(newfd);
				free(http_request);
				free(http_response);
				exit(EXIT_SUCCESS);

			default:		// Parent process
							// Parent doesn't need this fd
				close(newfd);
				wait(NULL);
		}
	}

	return 0;
}

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

int
parse_http_request(char buffer[BUFFER_SIZE], struct http_request *request) {
	if (sscanf(buffer, "%s %s %s",
			request->method,
			request->url,
			request->protocol_version) != 3) {
		return -1;
	}

	return 0;
}

int 
http_request_handle_method_get(struct http_request *request, struct http_response *response) {
	if (strcmp(request->method, "GET") == 0) {
		response->status_code = 200;
		return 0;
	}

	// 400 Bad Request
	// TODO: Do not exit, serve bad request instead
	response->status_code = 400;
	return -1;
}

int
http_request_handle_url(struct http_request *request, struct http_response *response) {
	// TODO: Handle rerouting
	if (strcmp(request->url, "/") == 0) {
		strcpy(response->url, "html/index.html");
		strcpy(response->content_type, "text/html");
		return 0;
	}
	// 400 Bad Request
	// TODO: Do not exit, serve bad request instead
	response->status_code = 400;
	return -1;
}

int
handle_client_http_request(char client_buffer_request[BUFFER_SIZE], 
		struct http_request * request,
		struct http_response * response) {
	
	// 1. Parse client request
	if (parse_http_request(client_buffer_request, request) == -1) {
		// Return -1
		fprintf(stderr, "[HTTP]: Failed to parse client request.\n");
	}
	// 2. Respond based on the request with a new struct response
	//		Because right now the code is manipulating the request struct data
	// 2. Respond to method
	http_request_handle_method_t func_ptr = &http_request_handle_method_get;
	if ((*func_ptr)(request, response) == -1) {
		// Return -1
		fprintf(stderr, "[HTTP]: Failed to handle method.");
	}
	
	// 3. Respond to URL by routing 
	if (http_request_handle_url(request, response) == -1) {
		// TODO: Handle rerouting
		fprintf(stderr, "[HTTP]: Failed to process URL.\n");
		// Return -1
		exit(EXIT_FAILURE);
	}

	strcpy(response->phrase, "OK");

	// Throw response status code errors if necessary eg 404 etc.
	// Finished parsing information from client
	return 0;
}
