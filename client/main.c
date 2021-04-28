#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_FDS 3
#define IN_MESSAGE_MAX 1000
#define OUT_MESSAGE_MAX 1000

struct pollfd pfds[NUM_FDS];

int is_verbose = 0;

static void verbose_log(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (is_verbose) {
    vprintf(fmt, args);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Format: client [hostname] [port].\n");
    exit(EXIT_FAILURE);
  }

  if (argc >= 4 && strcmp(argv[3], "-v") == 0) {
    is_verbose = 1;
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
  pfds[1].fd = 1;
  pfds[1].events = POLLOUT;
  pfds[2].fd = conn_fd;
  pfds[2].events = POLLIN | POLLOUT;

  int message_prepared = 0;
  int message_received = 0;

  uint8_t stdin_msg[IN_MESSAGE_MAX];
  ssize_t stdin_msg_len;
  uint8_t server_msg[IN_MESSAGE_MAX];
  ssize_t server_msg_len;
  (void)server_msg_len;

  /*uint8_t out_msg[IN_MESSAGE_MAX];*/
  while (1) {
    int num_events = poll(pfds, NUM_FDS, -1);
    if (num_events == -1) {
      fprintf(stderr, "Failed to poll file descriptors: %s.\n",
              strerror(errno));
    }

    /* stdin */
    if (pfds[0].revents & POLLIN) {
      stdin_msg_len = read(0, stdin_msg, IN_MESSAGE_MAX);
      message_prepared = 1;
      verbose_log("Read message from stdin.\n");
    }
    if ((pfds[1].revents & POLLOUT) && message_received) {
      write(1, server_msg, server_msg_len);
      message_received = 0;
    }
    /* server */
    if (pfds[2].revents & POLLIN) {
      server_msg_len = recv(pfds[2].fd, server_msg, IN_MESSAGE_MAX, 0);
      if (server_msg_len == 0) {
        verbose_log("Connection with server lost.\n");
        goto server_closed;
      } else if (server_msg_len == -1) {
        printf("Error reading from server: %s.\n", strerror(errno));
        goto server_closed;
      }
      verbose_log("Read message from server %d.\n", server_msg_len);
      message_received = 1;
    }
    if ((pfds[2].revents & POLLOUT) && message_prepared) {
      send(pfds[2].fd, stdin_msg, stdin_msg_len, 0);
      message_prepared = 0;
      verbose_log("Sending message to server.\n");
    }
  }

server_closed:

  close(conn_fd);

  freeaddrinfo(server_info);
}
