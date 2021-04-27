#define _POSIX_C_SOURCE 1
#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <paths.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_USED 4433

static int create_socket(int port) {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET; /* IPv4 */
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int fd = socket(addr.sin_family, SOCK_STREAM, 0);
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

/* Associates standard file descriptor with /dev/null */
static int open_devnull(int fd) {
  FILE *f = 0;
  if (fd == 0) f = freopen(_PATH_DEVNULL, "rb", stdin);
  if (fd == 1) f = freopen(_PATH_DEVNULL, "wb", stdout);
  if (fd == 2) f = freopen(_PATH_DEVNULL, "wb", stderr);
  return (f && fileno(f) == fd);
}

/* Closes all file descriptors > 3, and pipes 0, 1, 2 to /dev/null if
 * they are invalid */
static void clean_file_descriptors(void) {
  int fd, fds;
  struct stat st;

  /* If it returns -1, there are more open file descriptors than can be stored
   */
  if ((fds = getdtablesize()) == -1) fds = FOPEN_MAX;

  /* Starting from the descriptor after stderr, close every open file descriptor
   */
  for (fd = 3; fd < fds; fd++) {
    close(fd);
  }

  for (fd = 0; fd < 3; fd++) {
    if (fstat(fd, &st) == -1 && (errno != EBADF || !open_devnull(fd))) {
      /* Unable to get information about the default file descriptors, abort the
       * proram before security vunerabilities appear */
      abort();
    }
  }
}

/* If the program crashes, the memory dump files could be written, possibly
 * exposing confidential info */
static void disable_core_dumps(void) {
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
}

int main() {
  disable_core_dumps();

  clean_file_descriptors();

  int server_fd = create_socket(PORT_USED);

  const char reply[] = "beans\n";

  int client1_fd = accept(server_fd, NULL, 0);
  if (client1_fd < 0) {
    fprintf(stderr, "Unable to accept connection on port %d: %s.\n", PORT_USED,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  int client2_fd = accept(server_fd, NULL, 0);
  if (client2_fd < 0) {
    fprintf(stderr, "Unable to accept connection on port %d: %s.\n", PORT_USED,
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  send(client1_fd, reply, strlen(reply), 0);
  send(client2_fd, reply, strlen(reply), 0);

  struct pollfd pfds[2];
  size_t num_fds = 2;
  pfds[0].fd = client1_fd;
  pfds[0].events = POLLIN;
  pfds[1].fd = client2_fd;
  pfds[1].events = POLLIN;

#define MSG_BUF_SZ 1000
  uint8_t msg_buf[MSG_BUF_SZ];

  while (1) {
    int num_events = poll(pfds, 2, -1);

    if (num_events == -1) {
      fprintf(stderr, "Unable to poll file descriptors: %s.\n",
              strerror(errno));
      exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 2; i++) {
      if (pfds[i].revents & POLLIN) {
        size_t bytes_read = recv(pfds[i].fd, msg_buf, MSG_BUF_SZ, 0);
        if (bytes_read) {
          for (size_t j = 0; j < num_fds; j++) {
            send(pfds[j].fd, msg_buf, bytes_read, 0);
          }
          printf("%.*s", (int)bytes_read, msg_buf);
        }
      }
    }
  }

  close(client1_fd);
  close(client2_fd);

  close(server_fd);
  return 0;
}
