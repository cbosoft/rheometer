#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

const char *so_get_str(void *handle, const char *name)
{
  dlerror();
  const char **rv = dlsym(handle, name);
  char *error = NULL;
  if ((error = dlerror()) != NULL) {
    warn("load", "%s", error);
    return NULL;
  }

  return *rv;
}


ControllerHandle *load_controller_path(const char *path)
{
  ControllerHandle *h = calloc(1, sizeof(ControllerHandle));

  h->handle = dlopen(path, RTLD_LAZY);
  if (!h->handle) {
    ferr("load_controller", "%s", dlerror());
  }

  h->doc = so_get_str(h->handle, "doc");
  h->name = so_get_str(h->handle, "name");
  h->ident = so_get_str(h->handle, "ident");
  char *error = NULL;

  dlerror();
  int *n_params = dlsym(h->handle, "n_params");
  if ((error = dlerror()) != NULL) {
    warn("load_controller", " Could not load controller number params: %s", error);
    h->n_params = 0;
  }
  else {

    h->n_params = (*n_params);
    const char **params = dlsym(h->handle, "params");
    if ((error = dlerror()) != NULL) {
      warn("load_controller", "Could not load controller params doc: %s", error);
    }
    else {
      h->params = params;
    }
  }

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

  h->doc = so_get_str(h->handle, "doc");
  h->name = so_get_str(h->handle, "name");
  h->ident = so_get_str(h->handle, "ident");
  char *error = NULL;

  dlerror();
  int *n_params = dlsym(h->handle, "n_params");
  if ((error = dlerror()) != NULL) {
    warn("load_setter", " Could not load setter number params: %s", error);
    h->n_params = 0;
  }
  else {

    h->n_params = (*n_params);
    const char **params = dlsym(h->handle, "params");
    if ((error = dlerror()) != NULL) {
      warn("load_setter", "Could not load setter params doc: %s", error);
    }
    else {
      h->params = params;
    }
  }

  h->get_setpoint = dlsym(h->handle, "get_setpoint");
  if ((error = dlerror()) != NULL) {
    ferr("load_setter", "%s", error);
  }

  return h;
}
