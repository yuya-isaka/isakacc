#include "header.h"

char *format(char *fmt, ...) {
  char *buf;
  size_t size;
  FILE *out = open_memstream(&buf, &size);

  va_list ap;
  va_start(ap, fmt);
  vfprintf(out, fmt, ap);
  va_end(ap);
  fclose(out);
  return buf;
}