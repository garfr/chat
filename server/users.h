#pragma once

#include <poll.h>
#include <stdint.h>
#include <string.h>

#define MAX_USERNAME_LEN 50
#define MAX_USERS 50

typedef struct {
  struct pfds *sock;
  uint8_t name[MAX_USERNAME_LEN + 1];
} User;

User user_create(struct pfds *sock, const uint8_t name[MAX_USERNAME_LEN + 1]);

extern User user_list[50];
extern size_t num_users;

void user_list_init();
int user_list_add(User user);
int user_list_remove(size_t idx);
