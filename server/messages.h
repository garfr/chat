#pragma once

#include <stdint.h>

enum {
  MSG_PING,
  MSG_PONG,
  MSG_NICK,
  MSG_MSG,
};

typedef struct {
  int t;
  union {
    struct {
      uint8_t *nick;
      size_t nick_len;
    };
    struct {
      uint8_t *msg;
      size_t msg_len;
    };
  };
} Message;

int write_msg(Message *msg, int fd);
