#include "../include/utils.h"
#include "../include/server.h"

#include <getopt.h> // opterr, optarg, etc.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt(), STDOUT_FILENO

#define PROGRAM_NAME "Web Server"
#define PROGRAM_URL "https://github.com/mohafarman/web_server"
#define PROGRAM_VER_MAJOR 0
#define PROGRAM_VER_MINOR 1
#define PROGRAM_VER_PATCH 1

enum error_flag err_server = server, err_http = http;

void print_error(const char *error_msg, enum error_flag err_flag,
                 const char *file, int line) {
  char *flag;

  switch (err_flag) {
  case server:
    flag = "SERVER";
    break;
  case http:
    flag = "HTTP";
    break;
  }

  fprintf(stderr, "[%s]: %s - %s:%d\n", flag, error_msg, file, line);
}

/*
 * Parse command line args into the provided options_s struct.
 */
void parse_args(int argc, char **argv, options_s *opts) {
  opterr = 0;
  int o;
  while ((o = getopt(argc, argv, ":p:d:vh")) != -1) {
    switch (o) {
    case 'd':
      opts->root_dir = optarg;
      break;
    case 'p':
      opts->port = optarg;
      break;
    case 'h':
      opts->help = 1;
      break;
    case 'v':
      opts->version = 1;
      break;
    case ':': /* -p without operand */
      fprintf(stderr, "Option -%c requires an operand\n", optopt);
      break;
    case '?':
      fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
    }
  }
}

/*
 * Print usage information.
 */
void help(const char *invocation, FILE *where) {
  fprintf(where, "USAGE\n");
  fprintf(where, "\t%s [OPTIONS...]\n\n", invocation);
  fprintf(where, "OPTIONS\n");
  fprintf(where, "\t-b\tspecify port to be used\n");
  fprintf(where, "\t-d\troot directory to be served\n");
  fprintf(where, "\t-h\tprint this help text and exit\n");
  fprintf(where, "\t-v\tprint version information and exit\n");
}

/*
 * Print version information.
 */
void version(FILE *where) {
  fprintf(where, "%s %d.%d.%d\n%s\n", PROGRAM_NAME, PROGRAM_VER_MAJOR,
          PROGRAM_VER_MINOR, PROGRAM_VER_PATCH, PROGRAM_URL);
}

/*
 * Check to see if the root directory specified exists and has read, write
 * permissions
 * */
int root_dir_valid(char *root_dir_path) {
  // Check if folder exists
  if (access(root_dir_path, F_OK) == -1) {
    print_error("Root directory does not exist", err_server, __FILE__,
                __LINE__);
    return -1;
  }

  // Check if folder is readable
  if (access(root_dir_path, R_OK) == -1) {
    print_error("No read permission for root directory", err_server, __FILE__,
                __LINE__);
    return -1;
  }

  // Check if folder is writable
  if (access(root_dir_path, W_OK) == -1) {
    print_error("No write permission for root directory", err_server, __FILE__,
                __LINE__);
    return -1;
  }

  return 0;
}
