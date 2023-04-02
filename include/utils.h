#ifndef UTILS_H
#define UTILS_H

#include "server.h"

// int render_static_file(struct http_response *response);
char *render_static_file(const char *filename);

#endif /* UTILS_H */
