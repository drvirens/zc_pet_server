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

static void s_do_child_process(int listener, int worker_id)
{
  printf(" child started\n");
  pthread_setname_np("viren-child-thread");
  int kq = kqueue();
  struct kevent changes[1];
  struct kevent events[1];
  EV_SET(&changes[0], listener, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

  while (1) {
    //pthread_mutex_lock();
    int nevents = kevent(kq, changes, 1, events, 1, NULL);

    //pthread_mutex_unlock();

    pthread_t tid;

    for (int i = 0; i < nevents; i++) {
      printf("readable . i think now you accept not sure i=%d\n", i);
    } //end for
  } //end while

  printf("exitting child process\n");
}

static void s_parent_waits(void)
{
  int wstatus;
  while (waitpid(WAIT_ANY, &wstatus, WNOHANG) == 0) {
  }
  printf("exitting parent process\n");
}
#define ZC_MAX_WORKERS 1
static pid_t worker_process_pid[ZC_MAX_WORKERS];
static void s_kill_all_childs(void)
{
  for (int i = 0; i < ZC_MAX_WORKERS; i++) {
    pid_t kill_pid = worker_process_pid[i];
  }
}
static void s_spawn_workers(int listener, int num_workers)
{
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
extern void SERVER_start(void)
{
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

  //  socklen_t len = sizeof(sa);
  //  r = accept(listener, (struct sockaddr *)&sa, &len);
  //  if (-1 == r) {
  //    perror("accept error. reason ");
  //    exit(EXIT_FAILURE);
  //  }
}