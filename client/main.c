#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_FDS 2
#define IN_MESSAGE_MAX 1000
#define OUT_MESSAGE_MAX 1000
struct pollfd pfds[NUM_FDS];

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Format: client [hostname] [port].\n");
    exit(EXIT_FAILURE);
  }
  struct addrinfo hints;
  struct addrinfo* server_info;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  const char* domain_name = argv[1];
  if (strcmp(argv[1], "localhost") == 0) {
    argv[1] = NULL;
    hints.ai_flags = AI_PASSIVE;
  }

  int status;
  if ((status = getaddrinfo(argv[1], argv[2], &hints, &server_info)) != 0) {
    fprintf(stderr, "Cannot error getting address info: %s.\n",
            gai_strerror(status));
    exit(EXIT_FAILURE);
  }

  int conn_fd = socket(server_info->ai_family, server_info->ai_socktype,
                       server_info->ai_protocol);
  if (conn_fd == -1) {
    fprintf(stderr, "Cannot open socket at %s:%s.\n", domain_name, argv[2]);
    exit(EXIT_FAILURE);
  }

  if (connect(conn_fd, server_info->ai_addr, server_info->ai_addrlen) == -1) {
    fprintf(stderr, "Cannot open connection at %s:%s.\n", domain_name, argv[2]);
    exit(EXIT_FAILURE);
  }

  pfds[0].fd = 0;
  pfds[0].events = POLLIN;
  pfds[1].fd = conn_fd;
  pfds[1].events = POLLIN | POLLOUT;

  int has_message = 0;
  uint8_t stdin_msg[IN_MESSAGE_MAX];
  size_t stdin_msg_len;
  uint8_t server_msg[IN_MESSAGE_MAX];
  size_t server_msg_len;
  (void)server_msg_len;

  /*uint8_t out_msg[IN_MESSAGE_MAX];*/
  while (1) {
    int num_events = poll(pfds, NUM_FDS, -1);
    if (num_events == -1) {
      fprintf(stderr, "Failed to poll file descriptors: %s.\n",
              strerror(errno));
    }

    /* stdin */
    if (pfds[0].events & POLLIN) {
      stdin_msg_len = read(0, stdin_msg, IN_MESSAGE_MAX);
      has_message = 1;
    }
    /* server */
    if (pfds[1].events & POLLIN) {
      server_msg_len = recv(pfds[1].fd, server_msg, IN_MESSAGE_MAX, 0);
    }
    if (pfds[1].events & POLLOUT && has_message) {
      send(pfds[1].fd, stdin_msg, stdin_msg_len, 0);
      has_message = 0;
    }
  }

  const char* msg = "beans\n";
  while (1) {
    ssize_t res = send(conn_fd, msg, sizeof(msg), 0);
    if (res <= 0) {
      printf("%zd.\n", res);
      break;
    }
  }
  printf("Hello, client.\n");

  close(conn_fd);

  freeaddrinfo(server_info);
}
