#pragma once




#include "run.h"




#define BOLD  "\033[1m"
#define RESET "\033[0m"
// FOREGROUND
#define FGBLACK   "\033[30m"
#define FGRED     "\033[31m"
#define FGGREEN   "\033[32m"
#define FGYELLOW  "\033[33m"
#define FGBLUE    "\033[34m"
#define FGMAGENTA "\033[35m"
#define FGCYAN    "\033[36m"
#define FGWHITE   "\033[37m"
// BACKGROUND
#define BGBLACK   "\033[40m"
#define BGRED     "\033[41m"
#define BGGREEN   "\033[42m"
#define BGYELLOW  "\033[43m"
#define BGBLUE    "\033[44m"
#define BGMAGENTA "\033[45m"
#define BGCYAN    "\033[46m"
#define BGWHITE   "\033[47m"




void centre(char *s, unsigned int w, char **c);
void display_thread_data(struct run_data *td);
void display_titles();




// vim: ft=c
