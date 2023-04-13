#ifndef UTILS_H
#define UTILS_H

#include "server.h"

enum error_flag { server, http };

void print_error(const char *error_msg, enum error_flag err_flag,
                 const char *file, int line);

#endif /* UTILS_H */
