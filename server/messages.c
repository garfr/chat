#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "messages.h"

static const char *msg_type_bufs[] = {
    "PING",
    "PONG",
    "NICK ",
    "MSG ",
};

static uint8_t temp_buf[1000];
size_t buf_len;

static void prepare_msg_buffer(Message *msg) {
  buf_len = strlen(msg_type_bufs[msg->t]);

  memcpy(temp_buf, msg_type_bufs[msg->t], buf_len);
  temp_buf[buf_len++] = ' ';

  switch (msg->t) {
    case MSG_PING:
      return;
    case MSG_PONG:
      return;
    case MSG_NICK:
      memcpy(&temp_buf[buf_len], msg->nick, msg->nick_len);
      buf_len += msg->nick_len;
      return;
    case MSG_MSG:
      memcpy(&temp_buf[buf_len], msg->msg, msg->msg_len);
      buf_len += msg->msg_len;
      return;
    default:
      fprintf(stderr, "Invalid message type: %d.\n", msg->t);
      exit(EXIT_FAILURE);
  }
}
int write_msg(Message *msg, int fd) {
  prepare_msg_buffer(msg);
  return send(fd, temp_buf, buf_len, 0) != -1;
}
