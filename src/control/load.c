#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "../util/error.h"
#include "control.h"


ControllerHandle *load_controller(const char *name)
{
  char *path = malloc(401*sizeof(char));
  snprintf(path, 400, "./controllers/%s.so", name);
  ControllerHandle *rv = load_controller_path(path);
  free(path);

  return rv;
}


ControllerHandle *load_controller_path(const char *path)
{
  ControllerHandle *h = calloc(1, sizeof(ControllerHandle));

  h->handle = dlopen(path, RTLD_LAZY);
  if (!h->handle) {
    ferr("load_controller", "%s", dlerror());
  }

  dlerror();
  char **doc_ptr = dlsym(h->handle, "doc");
  char *error = NULL;
  if ((error = dlerror()) != NULL) {
    ferr("load_controller", "%s", error);
  }
  h->doc = *doc_ptr;

  h->get_control_action = dlsym(h->handle, "get_control_action");
  if ((error = dlerror()) != NULL) {
    ferr("load_controller", "%s", error);
  }

  return h;
}


SetterHandle *load_setter(const char *name)
{
  char *path = malloc(401*sizeof(char));
  snprintf(path, 400, "./setters/%s.so", name);
  SetterHandle *rv = load_setter_path(path);
  free(path);
  return rv;
}


SetterHandle *load_setter_path(const char *path)
{
  SetterHandle *h = calloc(1, sizeof(SetterHandle));

  h->handle = dlopen(path, RTLD_LAZY);

  if (!h->handle) {
    ferr("load_setter", "%s", dlerror());
  }

  dlerror();
  char **doc_ptr = dlsym(h->handle, "doc");
  char *error = NULL;
  if ((error = dlerror()) != NULL) {
    ferr("load_setter", "%s", error);
  }
  h->doc = *doc_ptr;

  h->get_setpoint = dlsym(h->handle, "get_setpoint");
  if ((error = dlerror()) != NULL) {
    ferr("load_setter", "%s", error);
  }

  return h;
}
