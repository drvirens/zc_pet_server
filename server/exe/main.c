//
//  main.c
//  pet_server
//
//  Created by Virendra Shakya on 1/13/19.
//  Copyright © 2019 Virendra Shakya. All rights reserved.
//
#include "zc_srv.h"
#include <signal.h>
#include <stdio.h>

static int sig_piped_count_;
static void s_handle_sigpipe(int s)
{
  sig_piped_count_++;
}

int main(int argc, const char* argv[])
{
  printf("started parent process\n");
  signal(SIGPIPE, &s_handle_sigpipe);
  SERVER_start();
  return 0;
}
