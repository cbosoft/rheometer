#pragma once

typedef struct arglist {
  char **argv;
  int argc;
} ArgList;

ArgList *arglist_new();
void arglist_add(ArgList *al, const char *s);
ArgList *arglist_copy(ArgList *al);
void arglist_free(ArgList *al);


typedef struct argset {
  ArgList **vargv;
  int margc;
} ArgSet;

ArgSet *argset_new();
void argset_add(ArgSet *as, ArgList *al);
void argset_free(ArgSet *as);
void argset_add_head_tail(ArgSet **as, ArgList *head, ArgList *tail);

