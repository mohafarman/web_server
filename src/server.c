#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // close

#include "../include/server.h"
#include "../include/utils.h"

#define PROGRAM_NAME "web_server"
#define ENABLE_DEBUG 1 // Enables debug code if set to 1

enum error_flag err_flag_server = server, err_flag_http = http;

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

  /*
   * Look at the fakesteak.c file and adopt the same way of
   * argument handling as he does
   */

  /* TODO: Let the user determine root directory from an argument */

  // parse command line options
  options_s opts = {0};
  parse_args(argc, argv, &opts);

  if (opts.help) {
    help(argv[0], stdout);
    return EXIT_SUCCESS;
  }

  if (opts.version) {
    version(stdout);
    return EXIT_SUCCESS;
  }

  if (opts.port == NULL) {
    opts.port = "4001";
    fprintf(stdout, "Using default port %s\n", opts.port);
  }

  if (opts.root_dir == NULL) {
    opts.root_dir = "html/";
    fprintf(stdout, "Using root directory %s\n", opts.root_dir);
  }

  /* Check so that the root directory specified exists and we have read
   * write permissions */
  if (root_dir_valid(opts.root_dir) == -1) {
    print_error("Root directory failed.", err_flag_server, __FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE;     // use my IP

  if (getaddrinfo(NULL, opts.port, &hints, &results) != 0) {
    char *buff[256];
    asprintf(buff, "getaddrinfo: %s", gai_strerror(return_value));
    print_error(*buff, err_flag_server, __FILE__, __LINE__);
  }

  // Loop through all the results and bind to the first we can
  for (p = results; p != NULL; p = p->ai_next) {

    // Attempt to open a TCP network connection throw error
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("Server: socket\n");
      continue;
    }

    // Allow reuse of the port if the socket is still "hanging around" in the
    // kernel after previous use
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
    print_error("Server: Failed to bind to socket.\n", err_flag_server,
                __FILE__, __LINE__);
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
  printf("Server: waiting for connections on port %s...\n", opts.port);

  while (1) {

    sin_size = sizeof their_addr;

    // Program blocks on accept until a connection has been made
    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (newfd == -1) {
      perror("Server: accept\n");
      exit(EXIT_FAILURE);
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              str_inet, sizeof str_inet);

    printf("Server: got connection from %s:%d\n", str_inet,
           get_port(their_addr));

    if ((pid = fork()) == -1) {
      perror("Fork failed\n");
    }

    switch (pid) {
    case 0: // Child process
      close(sockfd);
      struct http_request *http_request = malloc(sizeof(struct http_request));
      struct http_response *http_response =
          malloc(sizeof(struct http_response));

      /* "Append" the root directory to the response */
      strcpy(http_response->url, opts.root_dir);

      // Read in the client request
      if (read(newfd, client_buffer_request, BUFFER_SIZE) == -1) {
        perror("Server: read\n");
      }

      printf("========\nClient request:\n%s\n========\n",
             client_buffer_request);

      // Parse and handle client HTTP request
      handle_client_http_request(client_buffer_request, http_request,
                                 http_response);

      if (send(newfd, http_response->final_response,
               strlen(http_response->final_response), 0) == -1) {
        perror("Server: send\n");
      }

      // fclose(index_html);
      close(newfd);
      free(http_request);
      free(http_response);
      exit(EXIT_SUCCESS);

    default: // Parent process
      // Parent doesn't need this fd
      close(newfd);
      wait(NULL);
    }
  }

  return 0;
}

void sigchld_handler(int signum) {

  (void)signum;
  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {

  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

uint16_t get_port(struct sockaddr_storage addr) {
  if (addr.ss_family == AF_INET) { // IPv4
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    return ntohs(s->sin_port);
  } else if (addr.ss_family == AF_INET6) { // IPv6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
    return ntohs(s->sin6_port);
  } else {
    print_error("Unknown address family\n", err_flag_server, __FILE__,
                __LINE__);
    return 0;
  }
}

int parse_http_request(char buffer[BUFFER_SIZE], struct http_request *request,
                       struct http_response *response) {
  if (sscanf(buffer, "%s %s %s", request->method, request->url,
             request->protocol_version) != 3) {
    response->status_code = 400;
    strcpy(response->phrase, "Bad Request");
    return -1;
  }

  strcpy(response->protocol_version, request->protocol_version);
  return 0;
}

int http_request_handle_method_get(struct http_request *request,
                                   struct http_response *response) {
  if (strcmp(request->method, "GET") == 0) {
    response->status_code = 200;
    return 0;
  }

  // TODO: Do not exit, serve bad request instead
  // 501 Not Implemented
  response->status_code = 501;
  strcpy(response->phrase, "Not Implemented");
  return -1;
}

int http_request_handle_url(struct http_request *request,
                            struct http_response *response) {
  /*
   * TODO: Handle rerouting
   * Use a hash function or bst here: url as a key and the file as a value
   * Alternatively try out the stb
   * */
  if (strcmp(request->url, "/") == 0) {
    /* Append the final directory/file to the root directory */
    strcat(response->url, "index.html");
    strcpy(response->content_type, "text/html");
    return 0;
  }

  // 400 Bad Request
  // TODO: Do not exit, serve bad request instead
  response->status_code = 400;
  return -1;
}

int handle_client_http_request(char client_buffer_request[BUFFER_SIZE],
                               struct http_request *request,
                               struct http_response *response) {

  // 1. Parse client request
  if (parse_http_request(client_buffer_request, request, response) == -1) {
    print_error("Failed to parse client request.\n", err_flag_http, __FILE__,
                __LINE__);
    goto error;
  }

  // 2. Respond to method
  http_request_handle_method_t func_ptr = &http_request_handle_method_get;
  if ((*func_ptr)(request, response) == -1) {
    print_error("Method yet not implemented.", err_flag_http, __FILE__,
                __LINE__);
    goto error;
  }

  // 3. Respond to URL by routing
  if (http_request_handle_url(request, response) == -1) {
    print_error("Failed to process URL.", err_flag_http, __FILE__, __LINE__);
    return -1;
  }

  // 4. TODO Read file content based on url

  if (read_file(response) == -1) {
    print_error("Failed to read in file content.\n", err_flag_http, __FILE__,
                __LINE__);
    return -1;
  }

  if (construct_http_header(response) == -1) {
    print_error("Failed to construct an HTTP header.\n", err_flag_http,
                __FILE__, __LINE__);
    return -1;
  }

  // 4. Set phrase
  strcpy(response->phrase, "OK");

  strcat(response->final_response, response->http_header);
  strcat(response->final_response, response->content);

  return 0;

error:
  if (construct_http_header(response) == -1) {
    print_error("Failed to construct an HTTP header.\n", err_flag_http,
                __FILE__, __LINE__);
    return -1;
  }

  strcat(response->final_response, response->http_header);
  return 0;
}

int read_file(struct http_response *response) {
  char file_buffer[BUFFER_SIZE];
  // Serve file based on url returned from the server
  FILE *fptr = fopen(response->url, "r");
  if (fptr == NULL) {
    print_error("Error opening index.html file\n", err_flag_server, __FILE__,
                __LINE__);
  }

  // Get size of file
  fseek(fptr, 0, SEEK_END);
  long file_size = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);

  // Read file into buffer
  size_t bytes_read = fread(file_buffer, sizeof(char), file_size, fptr);

  // set content length determining bytes read into memory from file
  // (same as file_size)
  response->content_length = (int)bytes_read;

  // Append the content (file content) to the response which includes the
  // HTTP header
  // strcat(response->final_response, response->http_header);
  strcat(response->content, file_buffer);
  return 0;
}

int construct_http_header(struct http_response *response) {
  if (response->status_code == 200) {
    // Construct the HTTP header
    if (sprintf(response->http_header,
                "%s %d %s\r\nServer: %s\r\nContent-type: %s\r\nContent-length: "
                "%d\r\n\r\n",
                response->protocol_version, response->status_code,
                response->phrase, PROGRAM_NAME, response->content_type,
                response->content_length) < 0) {
      print_error("Failed to construct a HTTP header.\n", err_flag_http,
                  __FILE__, __LINE__);
      return -1;
    }
  } else {
    if (sprintf(response->http_header, "HTTP/1.1 %d %s\r\nServer: %s\r\n\r\n",
                response->status_code, response->phrase, PROGRAM_NAME) < 0) {
      print_error("Failed to construct a HTTP header.\n", err_flag_http,
                  __FILE__, __LINE__);
      return -1;
    }
  }

  return 0;
}
