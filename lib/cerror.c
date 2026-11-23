#include <arstd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const cerror_entry_t *g_entries = NULL;
static size_t g_count = 0;

void cerror_register(const cerror_entry_t *entries, size_t count) {
  g_entries = entries;
  g_count = entries ? count : 0;
}

cerror_t cerror(const char *name) {
  if (!name || name[0] == '\0') {
    return CERROR_DEFAULT;
  }
  if (strcmp(name, "ERR_OK") == 0) {
    return CERROR_OK;
  }
  for (size_t i = 0; i < g_count; i++) {
    if (g_entries[i].name && strcmp(g_entries[i].name, name) == 0) {
      return (cerror_t){
          .code = (int)(i + 1),
          .name = g_entries[i].name,
          .message = g_entries[i].message,
      };
    }
  }
  if (strcmp(name, "ERR_DEFAULT") == 0) {
    return CERROR_DEFAULT;
  }
  return (cerror_t){
      .code = -1,
      .name = name,
      .message = "Unknown error",
  };
}

const char *cerror_strerror(cerror_t err) {
  if (err.code == 0) {
    return "Success";
  }
  return err.message ? err.message : "An error occurred";
}

const char *cerror_err_name(cerror_t err) {
  if (err.code == 0) {
    return "ERR_OK";
  }
  return err.name ? err.name : "ERR_DEFAULT";
}

void cerror_panic(cerror_t err, const char *msg, const char *file, int line) {
  if (msg) {
    fprintf(stderr, "panic at %s:%d: %s: [%s] %s\n", file, line, msg,
            cerror_err_name(err), cerror_strerror(err));
  } else {
    fprintf(stderr, "panic at %s:%d: unwrapped Err: [%s] %s\n", file, line,
            cerror_err_name(err), cerror_strerror(err));
  }
  abort();
}
