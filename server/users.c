#include "users.h"

User user_list[50];
size_t num_users;

User user_create(struct pfds* sock, const uint8_t name[MAX_USERNAME_LEN + 1]) {
  User ret = {.sock = sock};
  memcpy(ret.name, name, MAX_USERNAME_LEN + 1);
  return ret;
}

void user_list_init() { memset(user_list, 0, sizeof(User) * MAX_USERS); }

int user_list_add(User user) {
  if (num_users + 1 > MAX_USERS) {
    return 0;
  }
  user_list[num_users++] = user;
  return 1;
}

int user_list_remove(size_t idx) {
  if (idx >= num_users) {
    return 0;
  }
  user_list[idx] = user_list[num_users - 1];
  num_users--;
  return 1;
}
