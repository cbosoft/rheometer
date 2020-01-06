#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "run.h"
#include "error.h"
#include "log.h"
#include "util.h"

//ffmpeg -f video4linux2 -i /dev/v4l/by-id/usb-0c45_USB_camera-video-index0 -vframes 2 test%3d.jpeg
char *capture_args[20] = {
  "ffmpeg", "-y", //0 1
  "-f", "v4l2", //2 3
  "-vframes", "3", //4 5
  "-input_format", "mjpeg",//6 7
  "-video_size", "320x240", //8 9
  "-i", "/dev/video0", //10 11
  "-f", "jpeg", //12 13
  "out.jpg", NULL}; //14 15
static int INPUT_ARG = 11;
static int OUTPUT_ARG = 14;


void take_photo(struct run_data *rd)
{

  int photo_idx = add_log(rd, "photo", "%s_photo.mp4", rd->log_pref);
  capture_args[INPUT_ARG] = rd->photo_device;
  capture_args[OUTPUT_ARG] = rd->log_paths[photo_idx];

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
    execvp("ffmpeg", capture_args);
  }

  fcntl(sp_stderr[1], F_SETFL, O_NONBLOCK);

  info("ffmpeg forked (photo): PID=%d", child_pid);
  time_t now = time(NULL);
  rd->cam_ready = 1;
  rd->cam_start = now;

  int i = 0;
  while ( (!rd->stopped) && (!rd->errored) ) {

    if (waitpid(child_pid, NULL, WNOHANG)) {
      break;
    }

    sleep_ms(100);
    i++;

  }

}

