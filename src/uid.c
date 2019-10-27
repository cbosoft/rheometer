#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


#include "uid.h"
#include "run.h"

extern char *nouns[];
extern int number_nouns;

char *get_random_noun()
{
  return nouns[rand() % number_nouns];
}


char *get_uid()
{
  time_t now = time(NULL);
  
  srand(now);

  char *uid[3];
  uid[0] = get_random_noun();
  uid[1] = get_random_noun();
  uid[2] = get_random_noun();

  int uid_length = strlen(uid[0])+strlen(uid[1])+strlen(uid[2])+3+1;
  char *rv = calloc(uid_length, sizeof(char*));
  snprintf(rv, uid_length, "%s %s %s", uid[0], uid[1], uid[2]);
  return rv;
}

