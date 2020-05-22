#pragma once


void set_quiet();
void set_loud();
int get_quiet();

void ferr(const char *source, const char* fmt, ...);
void warn(const char *source, const char* fmt, ...);
void argerr(const char* fmt, ...);
void info(const char* fmt, ...);




// vim: ft=c
