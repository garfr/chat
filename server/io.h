#pragma once

#include <poll.h>

#define MAX_PFDS 1000

extern struct pollfd pfds[MAX_PFDS];

void init_net_io();
