#pragma once

#include <stdint.h>
#include <string.h>

#define MAX_USERNAME_LEN 50
#define MAX_USERS 50

typedef struct {
  struct pollfd *sock;
  uint8_t name[MAX_USERNAME_LEN];
  size_t len;
} User;

extern User user_list[50];
extern size_t num_users;

void user_list_init();
int user_list_add(struct pollfd *sock, const uint8_t *name, size_t len);
int user_list_remove(size_t idx);
