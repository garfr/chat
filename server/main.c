#define _POSIX_C_SOURCE 1
#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <msgpack.h>
#include <paths.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "io.h"
#include "messages.h"
#include "users.h"
#include "common/helpers.h"

#define MSG_BUF_SZ 1000

uint8_t msg_buf[MSG_BUF_SZ];
ssize_t msg_len;

/* Associates standard file descriptor with /dev/null */
static int open_devnull(int fd) {
  FILE *f = 0;
  if (fd == 0)
    f = freopen(_PATH_DEVNULL, "rb", stdin);
  if (fd == 1)
    f = freopen(_PATH_DEVNULL, "wb", stdout);
  if (fd == 2)
    f = freopen(_PATH_DEVNULL, "wb", stderr);
  return (f && fileno(f) == fd);
}

/* Closes all file descriptors > 3, and pipes 0, 1, 2 to /dev/null if
 * they are invalid */
static void clean_file_descriptors(void) {
  int fd, fds;
  struct stat st;

  /* If it returns -1, there are more open file descriptors than can be stored
   */
  if ((fds = getdtablesize()) == -1)
    fds = FOPEN_MAX;

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

static void scan_users() {
  for (size_t i = 0; i < num_users; i++) {
    if (user_list[i].sock->revents & POLLIN) {
      msg_len = io_get_input(user_list[i].sock->fd, msg_buf, MSG_BUF_SZ);
      if (msg_len == 0) {
        /* User has disconnected */
        verbose_log("Removed connection: %d.\n", user_list[i].sock->fd);
        user_list_remove(i);
      } else {
        verbose_log("Recieved message from %d: \"%.*s\".\n",
                    user_list[i].sock->fd, (int)msg_len - 1, msg_buf);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
    enable_verbose();
  }

  disable_core_dumps();
  verbose_log("Disabled core dumps.\n");

  clean_file_descriptors();
  verbose_log("Cleared all unwanted file descriptors.\n");

  init_net_io();
  user_list_init();

  verbose_log("Initialized file descriptor list.\n");

  while (1) {
    int num_events = poll(pfds, num_fds + RESERVED_FDS, -1);

    if (num_events == -1) {
      fprintf(stderr, "Unable to poll file descriptors: %s.\n",
              strerror(errno));
      exit(EXIT_FAILURE);
    }

    /* SIGINT has occured */
    if (pfds[1].revents & POLLIN) {
      goto done;
    }

    /* New connection */
    if (pfds[0].revents & POLLIN) {
      int new_conn = accept(pfds[0].fd, NULL, 0);
      if (new_conn == -1) {
        fprintf(stderr, "Failed to accept network connection.\n");
        continue;
      }
      struct pollfd *new_fd = io_add_conn(new_conn);

      if (!new_fd) {
        fprintf(stderr, "Failed to add new connection to pfds.\n");
      } else {
        user_list_add(new_fd, NULL, 0);
        verbose_log("Added new user: %d.\n", new_conn);
      }
      num_events--; /* This doesn't count as a true "event" */
    }

    if (num_events) {
      scan_users();
    }
  }
done:
  close(pfds[0].fd);
  return 0;
}
