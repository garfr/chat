#include "users.h"
#include "io.h"

User user_list[50];
size_t num_users;

void user_list_init() { memset(user_list, 0, sizeof(User) * MAX_USERS); }

int user_list_add(struct pollfd *sock, const uint8_t *name, size_t len) {
  if (num_users + 1 > MAX_USERS) {
    return 0;
  }
  user_list[num_users].sock = sock;
  if (name) {
    memcpy(user_list[num_users].name, name, len);
  }
  user_list[num_users++].len = len;
  return 1;
}

int user_list_remove(size_t idx) {
  if (idx >= num_users) {
    return 0;
  }
  io_remove_conn(user_list[idx].sock);
  user_list[idx] = user_list[num_users - 1];
  num_users--;
  return 1;
}
