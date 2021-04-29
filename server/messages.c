#include "messages.h"

#include <msgpack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int write_ping(int fd) {
  msgpack_sbuffer* buf = msgpack_sbuffer_new();

  msgpack_packer* pk = msgpack_packer_new(buf, msgpack_sbuffer_write);

  msgpack_pack_array(pk, 1);
  msgpack_pack_int(pk, MSG_PING);

  printf("Written ping to: %d.\n", fd);
  if (send(fd, buf->data, buf->size, 0) == -1) {
    return 0;
  }

  msgpack_packer_free(pk);
  msgpack_sbuffer_free(buf);
  return 1;
}
