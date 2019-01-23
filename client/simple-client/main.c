
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "zc_socket_path.h"

int main(int argc, const char* argv[])
{
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (0 == fd) {
    perror("could not create socket\n");
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  size_t len = sizeof(ZN_SOCKET_PATH);
  printf("socket path len is %ld\n", len);
  strncpy(addr.sun_path, ZN_SOCKET_PATH, sizeof(ZN_SOCKET_PATH));

  int connected = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
  if (-1 == connected) {
    perror("Could not connect boss: \n");
    exit(EXIT_FAILURE);
  }

  printf("hello this is test client for tcp ip communication\n");
  return 0;
}