#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

#include "run.h"
#include "error.h"
#include "log.h"


char *record_args[15] = {
  "ffmpeg", "-y", 
  "-f", "v4l2",
  "-i", "/dev/video0", 
  "-framerate", "15", 
  "-video_size", "640x480", 
  "-f", "mp4", "out.mp4", NULL};
const int INPUT_ARG = 5;
const int FRAMERATE_ARG = 7;
const int OUTPUT_ARG = 12;


void *cam_thread_func(void *vtd)
{
  struct run_data *rd = (struct run_data*)vtd;

  int video_idx = add_log(rd, "video", "%s_video.mp4", rd->log_pref);
  record_args[INPUT_ARG] = rd->video_device;
  record_args[OUTPUT_ARG] = rd->log_paths[video_idx];

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

  fprintf(stderr, "forked. child PID=%d\n", child_pid);

  rd->cam_ready = 1;

  int i = 0;
  while ( (!rd->stopped) && (!rd->errored) ) {

    if (waitpid(child_pid, NULL, WNOHANG)) {
      warn("cam_thread_func", "ffmpeg process died unexpectedly");
      pthread_exit(0);
    }

    sleep(1);
    i++;

  }

  if (!waitpid(child_pid, NULL, WNOHANG)) {
    write(sp_stdin[1], "q", 1);
    waitpid(child_pid, NULL, 0);
  }

  pthread_exit(0);

  return NULL;
}
