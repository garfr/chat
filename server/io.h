#pragma once

#include <poll.h>
#include <stddef.h>

#define MAX_PFDS 1000
#define RESERVED_FDS 2

extern struct pollfd pfds[MAX_PFDS + RESERVED_FDS];
extern size_t num_fds;

void init_net_io();

struct pollfd* io_add_conn(int fd);
