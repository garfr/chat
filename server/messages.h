#pragma once

#include <stdint.h>

enum {
  MSG_PING,
  MSG_PONG,
  MSG_MSG,
};

int write_ping(int fd);
