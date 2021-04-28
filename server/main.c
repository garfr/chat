#define _POSIX_C_SOURCE 1
#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <paths.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT_USED 4433
#define MSG_BUF_SZ 1000
#define POLL_MAX_FDS 10
#define RESERVED_FDS 2

uint8_t msg_buf[MSG_BUF_SZ];
struct pollfd pfds[POLL_MAX_FDS + RESERVED_FDS];
size_t num_fds;

static int create_socket(int port) {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET; /* IPv4 */
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int option = 1;
  int fd = socket(addr.sin_family, SOCK_STREAM, 0);
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

static void add_fd(int fd) {
  if (num_fds >= POLL_MAX_FDS) {
    printf("Max number of file descriptors reached.\n");
    exit(EXIT_FAILURE);
  }

  pfds[num_fds + RESERVED_FDS].fd = fd;
  pfds[num_fds + RESERVED_FDS].events = POLLIN;
  num_fds++;
}

static void scan_fd(size_t fd_index) {
  if (pfds[fd_index].revents & POLLIN) {
    size_t bytes_read = recv(pfds[fd_index].fd, msg_buf, MSG_BUF_SZ, 0);

    if (bytes_read) {
      for (size_t j = 1; j <= num_fds; j++) {
        if (j != fd_index) {
          send(pfds[j].fd, msg_buf, bytes_read, 0);
        }
      }
      printf("%.*s", (int)bytes_read, msg_buf);
    }
  }
}

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

static void initialize_pfds() {
  int server_fd = create_socket(PORT_USED);

  pfds[0].fd = server_fd;
  pfds[0].events = POLLIN;

  pfds[1].fd = disable_sigint();
  pfds[1].events = POLLIN;
}

int main() {
  disable_core_dumps();

  clean_file_descriptors();

  num_fds = 0;

  initialize_pfds();

  while (1) {
    int num_events = poll(pfds, num_fds + RESERVED_FDS, -1);

    if (num_events == -1) {
      fprintf(stderr, "Unable to poll file descriptors: %s.\n",
              strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (pfds[1].revents & POLLIN) {
      goto done;
    }
    if (pfds[0].revents & POLLIN) {
      add_fd(accept(pfds[0].fd, NULL, 0));
      num_events--;
    }

    if (num_events != 0) {
      for (size_t i = RESERVED_FDS; i <= num_fds + RESERVED_FDS; i++) {
        scan_fd(i);
      }
    }
  }
done:
  for (size_t i = RESERVED_FDS; i < num_fds + RESERVED_FDS; i++) {
    close(pfds[i].fd);
  }

  close(pfds[0].fd);
  return 0;
}
