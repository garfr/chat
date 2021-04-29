#include "io.h"

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT_USED 4433

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

static int create_server_socket(int port) {
  struct sockaddr_in6 addr;
  addr.sin6_family = AF_INET6; /* IPv6 */
  addr.sin6_port = htons(port);
  addr.sin6_addr = in6addr_any;

  int option = 1;
  int fd = socket(addr.sin6_family, SOCK_STREAM, 0);
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if (fd < 0) {
    fprintf(stderr, "Unable to open socket: %s.\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "Unable to bind to port %d: %s.\n", port, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (listen(fd, 1) < 0) {
    fprintf(stderr, "Unable to listen on port %d: %s.\n", port,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  return fd;
}

void init_net_io() {
  int server_fd = create_server_socket(PORT_USED);

  /* Poll the server_fd to check for new connections */
  pfds[0].fd = server_fd;
  pfds[0].events = POLLIN;

  /* Poll for SIGINT so it can be handled properly */
  pfds[1].fd = disable_sigint();
  pfds[1].events = POLLIN;
}
