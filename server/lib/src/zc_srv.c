#include "zc_socket_path.h"
#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRACE printf("%s\n", __PRETTY_FUNCTION__);
struct zc_srv {
  pthread_mutex_t acc_mutex;
  pthread_mutexattr_t acc_mutex_attr;
};
static struct zc_srv* g_srv;

static void s_read_conn(struct kevent ke, int worker_id)
{
  TRACE
  char rbuf[1024] = { 0 };
  int rbytes = read(ke.ident, rbuf, 1023);
  printf("READ => [%d] ", rbytes);
}
static void s_accept_conn(int listener, int kq)
{
  TRACE
  struct sockaddr_in c;
  socklen_t socklen = sizeof(c);
  int fd = accept(listener, (struct sockaddr*)&c, &socklen);
  if (-1 == fd) {
    perror("accept error. reason -> ");
    exit(EXIT_FAILURE);
  }
  char buf[255] = { 0 };
  socklen_t len = 255;
  inet_ntop(AF_INET, &c.sin_addr, buf, len);
  pid_t curr_pid = getpid();
  printf("connected -> %s in worker with pid to be : {%d}\n", buf, curr_pid);

  struct kevent changes_list[1];
  EV_SET(&changes_list[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
  kevent(kq, changes_list, 1, NULL, 0, NULL);
}
static void s_do_child_process(int listener, int worker_id)
{
  TRACE
  pthread_setname_np("viren-child-thread");
  int kq = kqueue();
  struct kevent changes[1];
  struct kevent events[1];
  EV_SET(&changes[0], listener, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

  while (1) {
    pthread_mutex_lock(&g_srv->acc_mutex);
    int nevents = kevent(kq, changes, 1, events, 1, NULL);
    pthread_mutex_unlock(&g_srv->acc_mutex);

    for (int i = 0; i < nevents; i++) {
      //printf("readable . i think now you accept not sure i=%d\n", i);
      struct kevent ke = events[i];
      if (ke.ident == listener) {
        s_accept_conn(listener, kq);
      } else {
        if (ke.filter == EVFILT_READ) {
          printf("read\n");
          s_read_conn(ke, worker_id);
        } else {
          printf("ERRORR--- UN EXECPECT6EDCALL BOSS\n");
        }
      }

    } //end for
  } //end while

  printf("exitting child process\n");
}

static void s_parent_waits(void)
{
  TRACE
  int wstatus;
  while (waitpid(WAIT_ANY, &wstatus, WNOHANG) == 0) {
  }
  printf("exitting parent process\n");
}
#define ZC_MAX_WORKERS 1
static pid_t worker_process_pid[ZC_MAX_WORKERS];
static void s_kill_all_childs(void)
{
  TRACE
  for (int i = 0; i < ZC_MAX_WORKERS; i++) {
    pid_t kill_pid = worker_process_pid[i];
  }
}
static void s_spawn_workers(int listener, int num_workers)
{
  TRACE
  if (num_workers)
    for (int i = 0; i < num_workers; i++) {
      pid_t pid = fork();
      if (-1 == pid) {
        perror("fork error. reason ");
        s_kill_all_childs();
        exit(EXIT_FAILURE);
      }

      if (0 == pid) { //child process
        s_do_child_process(listener, i);
        return;
      } else { //parent process
        worker_process_pid[i] = pid;
      }
    } //end for loop
  s_parent_waits();
}
static int s_create_shared_mem(void)
{
  TRACE
  g_srv = (struct zc_srv*)mmap(NULL, sizeof(*g_srv), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  if (!g_srv) {
    return -1;
  }
  memset(g_srv, 0, sizeof(*g_srv));
  pthread_mutexattr_init(&g_srv->acc_mutex_attr);
  pthread_mutexattr_setpshared(&g_srv->acc_mutex_attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&g_srv->acc_mutex, &g_srv->acc_mutex_attr);
  return 1;
}
extern void SERVER_start(void)
{
  TRACE
  s_create_shared_mem();

  int listener = socket(AF_UNIX, SOCK_STREAM, 0);
  if (-1 == listener) {
    perror("socket error. reason ");
    exit(EXIT_FAILURE);
  }
  static const char* p = ZN_SOCKET_PATH;

  struct sockaddr_un sa;
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path, p);
  sa.sun_len = strlen(p) + 1;

  int r = bind(listener, (const struct sockaddr*)&sa, sizeof(sa));
  if (-1 == r) {
    perror("bind error. reason ");
    exit(EXIT_FAILURE);
  }
  r = listen(listener, 10);
  if (-1 == r) {
    perror("listen error. reason ");
    exit(EXIT_FAILURE);
  }

  int num_workers = ZC_MAX_WORKERS;
  s_spawn_workers(listener, num_workers);
}