#pragma once

#include <poll.h>
#include <stdint.h>
#include <sys/types.h>

#define MAX_PFDS 1000
#define RESERVED_FDS 2

extern struct pollfd pfds[MAX_PFDS + RESERVED_FDS];
extern size_t num_fds;

void init_net_io();

struct pollfd *io_add_conn(int fd);
void io_remove_conn(struct pollfd *sock);

ssize_t io_get_input(int fd, uint8_t *buf, size_t buf_sz);
ssize_t io_write_output(int fd, const uint8_t *buf, size_t buf_sz);
