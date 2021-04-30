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
  json_object_clear(obj);
  json_decref(obj);
  return ret;
}

Message decode_message(const uint8_t *msg, size_t len) {
  json_error_t err;
  json_t *obj = json_loads((const char *)msg, len, &err);

  if (obj) {
    if (!json_is_array(obj)) {
      fprintf(stderr, "Expected array JSON object.\n");
      exit(EXIT_FAILURE);
    }

    json_t *type = json_array_get(obj, 0);
    if (!json_is_integer(type)) {
      fprintf(stderr, "Expected message type.\n");
      exit(EXIT_FAILURE);
    }

    int t = json_integer_value(type);
    json_decref(type);

    Message ret;
    ret.t = t;
    switch (t) {
      case MSG_PING:
      case MSG_PONG:
        break;
      case MSG_MSG:
        {
          json_t *msg = json_array_get(obj, 1);
          if (!json_is_string(msg)) {
            fprintf(stderr, "Expected message type.\n");
            exit(EXIT_FAILURE);
          }
          const uint8_t *temp_str = (const uint8_t *)json_string_value(msg);
          size_t len = json_string_length(msg);
          uint8_t *new_str = calloc(len, sizeof(uint8_t));
          memcpy(new_str, temp_str, len);
          ret.msg = new_str;
          ret.msg_len = len;
        }
    }
    return ret;
  } else {
    fprintf(stderr, "Invalid JSON received.\n");
    exit(EXIT_FAILURE);
  }
}
