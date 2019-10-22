#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#include "run.h"
char *record_args[9] = {"-f", "v4l2", "-framereate", "15", "-video_size", "640x480", "-i", "/dev/video0", "out.mkv"};
const int FRAMERATE_ARG = 3;
const int INPUT_ARG = 7;
const int OUTPUT_ARG = 8;


void *cam_thread_func(void *vtd)
{
  struct run_data *rd = (struct run_data*)vtd;

  char video_path[200] = {0};
  snprintf(video_path, 199, "%s_video.mkv", rd->log_pref);
  record_args[OUTPUT_ARG] = video_path;

  int sp_stdin[2], sp_stdout[2], sp_stderr[2];

  pipe(sp_stdin);
  pipe(sp_stdout);
  pipe(sp_stderr);

  int child_pid = fork();

  if (child_pid) {
    close(sp_stdin[0]);
    close(sp_stdout[1]);
    close(sp_stderr[1]);
  }
  else {
    close(sp_stdin[1]);
    close(sp_stdout[0]);
    close(sp_stderr[0]);

    dup2(sp_stdin[0], STDIN_FILENO);
    dup2(sp_stdout[1], STDOUT_FILENO);
    dup2(sp_stderr[1], STDERR_FILENO);
    execvp("ffmpeg", record_args);
  }

  rd->cam_ready = 1;

  while ( (!rd->stopped) && (!rd->errored) ) {

    // TODO: ping to ensure video is still running
    // check PID?

  }

  // send 'q\n' to stdin of child to kill cam
  // wait for it to process
  // check PID?

  pthread_exit(0);

  return NULL;
}
