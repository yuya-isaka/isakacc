#include "header.h"

char *format(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  char *buf;
  size_t buflen;
  FILE *out = open_memstream(&buf, &buflen);

  vfprintf(out, fmt, ap);
  va_end(ap);
  fclose(out);
  return buf;
}