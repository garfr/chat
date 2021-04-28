#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

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

  printf("Hello, client.\n");

  freeaddrinfo(server_info);
}
