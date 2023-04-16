#ifndef UTILS_H
#define UTILS_H

#include "server.h"
#include <stdio.h>

enum error_flag { server, http };

typedef struct options {
  char *port;          // port number to be used
  char *root_dir;      // root directory served
  uint8_t help : 1;    // show help and exit
  uint8_t version : 1; // show version and exit
} options_s;

void print_error(const char *error_msg, enum error_flag err_flag,
                 const char *file, int line);

void parse_args(int argc, char **argv, options_s *opts);

void help(const char *invocation, FILE *where);

void version(FILE *where);

int root_dir_valid(char *root_dir_path);

#endif /* UTILS_H */
