#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "run.h"
#include "error.h"
#include "log.h"


char *record_args[20] = {
  "ffmpeg", "-y", //1
  "-f", "v4l2", //3
  "-framerate", "10", //5
  "-input_format", "mjpeg",//7
  "-video_size", "320x240", //9
  "-i", "/dev/video0", //11
  "-f", "mp4", //13
  "out.mp4", NULL};
const int FRAMERATE_ARG = 5;
const int INPUT_ARG = 11;
const int OUTPUT_ARG = 14;


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

  info("ffmpeg forked: PID=%d", child_pid);
  fcntl(sp_stderr[1], F_SETFL, O_NONBLOCK);

  rd->cam_ready = 1;

  int i = 0;
  while ( (!rd->stopped) && (!rd->errored) ) {

    if (waitpid(child_pid, NULL, WNOHANG)) {
      // TODO: remove video from list of logs
      warn("cam_thread_func", "ffmpeg process died unexpectedly:");
      char *se = malloc((100 + 1)*sizeof(char));
      int c = read(sp_stderr[0], se, 100);
      while (c) {
        warn("cam_thread_func", "%s", se);
        c = read(sp_stderr[0], se, 100);
      }
      free(se);
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
