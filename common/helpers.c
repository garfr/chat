#include <stdarg.h>
#include <stdio.h>

#include "helpers.h"

static int is_verbose = 0;

void verbose_log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (is_verbose) {
    vprintf(fmt, args);
  }
}

void enable_verbose() { is_verbose = 1; }
