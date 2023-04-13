#include "../include/utils.h"
#include "../include/server.h"

#include <stdio.h>

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
