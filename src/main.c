#define _POSIX_C_SOURCE 1
#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    fprintf(stderr, "Unable to open socket.\n");
    exit(EXIT_FAILURE);
  }

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "Unable to bind to port %d.\n", port);
    exit(EXIT_FAILURE);
  }

  int yes = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (listen(fd, 1) < 0) {
    fprintf(stderr, "Unable to listen on port %d.\n", port);
    exit(EXIT_FAILURE);
  }

  return fd;
}

/* Associates standard file descriptor with /dev/null */
static int open_devnul(int fd) {
  FILE *f = 0;
  if (fd == 0) f = freopen(_PATH_DEVNULL, "rb", stdin);
  if (fd == 1) f = freopen(_PATH_DEVNULL, "wb", stdout);
  if (fd == 2) f = freopen(_PATH_DEVNULL, "wb", stderr);
  return (f && fileno(f) == fd);
}

/* Closes all file descriptors not needed, and pipes 0, 1, 2 to /dev/null if
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
    if (fstat(fd, &st) == -1 && (errno != EBADF || !open_devnul(fd))) {
      /* Unable to get information about the default file descriptors, abort the
       * proram before security vunerabilities appear */
      abort();
    }
  }
}

int main() {
  clean_file_descriptors();

  int sock_fd = create_socket(PORT_USED);

  for (;;) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    const char reply[] = "beans\n";

    int client_fd = accept(sock_fd, (struct sockaddr *)&addr, &len);
    if (client_fd < 0) {
      fprintf(stderr, "Unable to accept connection on port %d.\n", PORT_USED);
      exit(EXIT_FAILURE);
    }

    write(client_fd, reply, strlen(reply));
    close(client_fd);
  }

  close(sock_fd);

  return 0;
}
