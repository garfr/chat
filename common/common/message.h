#pragma once

#include <stdint.h>

enum {
  MSG_PING,
  MSG_PONG,
  MSG_MSG,
};

typedef struct {
  int t;
  union {
    struct {
      const uint8_t *msg;
      size_t msg_len;
    };
  };
} Message;

/* Encodes the message into a JSON object */
const uint8_t *encode_message(Message *msg);
Message decode_message(const uint8_t *msg, size_t len);
