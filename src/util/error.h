#pragma once

void set_silent();
void set_quiet();
void set_loud();

int get_silent();
int get_quiet();
int get_loud();

void ferr(const char *source, const char* fmt, ...);
void warn(const char *source, const char* fmt, ...);
void argerr(const char* fmt, ...);
void info(const char* fmt, ...);




// vim: ft=c
