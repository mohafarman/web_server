#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h> // SOMAXCONN
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 2048
#define MAX_BUFFER_SIZE 8192

// Used for parsing the client http request
struct http_request {
  char method[BUFFER_SIZE];
  char url[BUFFER_SIZE];
  char protocol_version[BUFFER_SIZE];
  char phrase[BUFFER_SIZE];
  int status_code;
  char content_type[BUFFER_SIZE];
};

// Used for constructing a server http response
struct http_response {
  char protocol_version[BUFFER_SIZE];
  int status_code;
  char phrase[BUFFER_SIZE];
  char url[BUFFER_SIZE];
  char http_header[BUFFER_SIZE * 3];
  char content_type[BUFFER_SIZE];
  char content[BUFFER_SIZE];
  int content_length;
  char final_response[MAX_BUFFER_SIZE];
};

void sigchld_handler(int signum);

void *get_in_addr(struct sockaddr *sa);

uint16_t get_port(struct sockaddr_storage addr);

int parse_http_request(char buffer[BUFFER_SIZE], struct http_request *request,
                       struct http_response *response);

int handle_client_http_request(char client_buffer_request[BUFFER_SIZE],
                               struct http_request *request,
                               struct http_response *response);

// Function pointer declaration to handle HTTP methods from client
typedef int (*http_request_handle_method_t)(struct http_request *,
                                            struct http_response *);

int http_request_handle_method_get(struct http_request *request,
                                   struct http_response *response);

int http_request_handle_url(struct http_request *request,
                            struct http_response *response);

int read_file(struct http_response *response);

int construct_http_header(struct http_response *response);

#endif /* SERVER_H */
