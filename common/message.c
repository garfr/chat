#include <jansson.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "message.h"

const uint8_t *encode_message(Message *msg) {
  json_t *obj;
  switch (msg->t) {
    case MSG_PING:
      obj = json_pack("[i]", MSG_PING);
      break;
    case MSG_PONG:
      obj = json_pack("[i]", MSG_PONG);
      break;
    case MSG_MSG:
      obj = json_pack("[is%]", MSG_MSG, msg->msg, msg->msg_len);
      break;
    default:
      fprintf(stderr, "Invalid message type: %d.\n", msg->t);
      exit(EXIT_FAILURE);
  }
  const uint8_t *ret = (uint8_t *)json_dumps(obj, 0);
  json_array_clear(obj);
  json_decref(obj);
  return ret;
}

Message decode_message(const uint8_t *msg, size_t len) {
  json_error_t err;
  json_t *obj = json_loadb((const char *)msg, len, 0, &err);
  if (obj == NULL) {
    printf("Invalid json object: \"%s\"\n", err.text);
  }

  Message ret = {0};
  if (json_unpack(obj, "[i]", &ret.t) == 0) {
    json_decref(obj);
    return ret;
  }
  const uint8_t *temp_str;
  if (json_unpack(obj, "[is%]", &ret.t, &temp_str, &ret.msg_len) == 0) {
    ret.msg = calloc(ret.msg_len, sizeof(uint8_t));
    memcpy((uint8_t *)ret.msg, temp_str, len);
    json_decref(obj);
    return ret;
  } else {
    fprintf(stderr, "Invalid json message.\n");
    exit(EXIT_FAILURE);
  }
}
