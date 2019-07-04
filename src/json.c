#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "rheo.h"



cJSON *read_json(const char* path)
{
  if (access(path, R_OK | F_OK) == -1) {
    ferr("read_json", "Could not read JSON file \"%s\".", path);
  }

  FILE *fp = fopen(path, "r");

  if (fp == NULL)
    ferr("read_json", "Could not read JSON file \"%s\".", path);
  
  char ch;
  unsigned int count = 0, i = 0;

  while ( (unsigned char)(ch = fgetc(fp)) != (unsigned char)EOF ) {
    count++;
  }

  if (fseek(fp, 0L, SEEK_SET) != 0)
    ferr("read_json", "something went wrong repositioning file.");

  char *json_str = calloc(count+1, sizeof(char));
  while ( (unsigned char)(ch = fgetc(fp)) != (unsigned char)EOF) {
    json_str[i] = ch;
    i++;
  }

  fclose(fp);

  cJSON *json = cJSON_Parse(json_str);
  free(json_str);

  return json;
}
