#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "argset.h"

ArgList *arglist_new()
{
  ArgList *rv = malloc(sizeof(ArgList));
  rv->argv = NULL;
  rv->argc = 0;
  return rv;
}

void arglist_add(ArgList *al, const char *s)
{
  al->argc++;
  al->argv = realloc(al->argv, al->argc*sizeof(char *));
  al->argv[al->argc-1] = strdup(s);
}

void arglist_free(ArgList *al)
{
  for (int i = 0; i < al->argc; i++) {
    free(al->argv[i]);
  }
  free(al->argv);
}

ArgSet *argset_new()
{
  ArgSet *rv = malloc(sizeof(ArgSet));
  rv->vargv = NULL;
  rv->margc = 0;
  return rv;
}

ArgList *arglist_copy(ArgList *al)
{
  ArgList *rv = arglist_new();
  for (int i = 0; i < al->argc; i++) {
    arglist_add(rv, al->argv[i]);
  }
  return rv;
}

void argset_add(ArgSet *as, ArgList *al)
{
  as->margc++;
  as->vargv = realloc(as->vargv, as->margc*sizeof(ArgList));
  as->vargv[as->margc-1] = arglist_copy(al);
}

void argset_free(ArgSet *as)
{
  for (int i = 0; i < as->margc; i++) {
    arglist_free(as->vargv[i]);
  }
  free(as);
}

void argset_add_head_tail(ArgSet **out, ArgList *head, ArgList *tail)
{
  ArgSet *rv = argset_new();
  ArgSet *as = (*out);

  for (int i = 0; i < as->margc; i++) {
    ArgList *toadd = arglist_new();
    for (int j = 0; j < head->argc; j++) {
      arglist_add(toadd, head->argv[j]);
    }
    for (int j = 0; j < as->vargv[i]->argc; j++) {
      arglist_add(toadd, as->vargv[i]->argv[j]);
    }
    for (int j = 0; j < tail->argc; j++) {
      arglist_add(toadd, tail->argv[j]);
    }

    argset_add(rv, toadd);
    arglist_free(toadd);
  }

  argset_free(as);
  (*out) = rv;
}
