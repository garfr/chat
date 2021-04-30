#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <stdio.h>
#include <poll.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>

#include "common/message.h"
#include "common/helpers.h"

#define NUM_FDS 4
#define IN_MESSAGE_MAX 1000
#define OUT_MESSAGE_MAX 1000

struct pollfd pfds[NUM_FDS];

static int disable_sigint() {
  sigset_t sig_mask;

  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGINT);

  if (sigprocmask(SIG_BLOCK, &sig_mask, NULL) == -1) {
    fprintf(stderr, "Unable to block sig int handler.\n");
    exit(EXIT_FAILURE);
  }

  int sigint_fd = signalfd(-1, &sig_mask, 0);
  if (sigint_fd == -1) {
    fprintf(stderr, "Unable to create file descriptor from signal mask: %s.\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  return sigint_fd;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Format: client [hostname] [port].\n");
    exit(EXIT_FAILURE);
  }

  if (argc >= 4 && strcmp(argv[3], "-v") == 0) {
    enable_verbose();
  }

  struct addrinfo hints;
  struct addrinfo *server_info;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  const char *domain_name = argv[1];
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
  pfds[3].fd = disable_sigint();
  pfds[3].events = POLLIN;

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
      verbose_log("Read message from server %d: \"%.*s\".\n", pfds[2].fd,
                  (int)server_msg_len, server_msg);
      Message msg = decode_message(server_msg, server_msg_len);
      (void)msg;
    }
    if ((pfds[2].revents & POLLOUT) && message_prepared) {
      send(pfds[2].fd, stdin_msg, stdin_msg_len, 0);
      message_prepared = 0;
      verbose_log("Sending message to server.\n");
    }
    if ((pfds[3].revents & POLLIN)) {
      verbose_log("SIGINT polled and caught.\n");
      goto server_closed;
    }
  }

server_closed:

  close(conn_fd);

  printf("here\n");
  freeaddrinfo(server_info);
}
