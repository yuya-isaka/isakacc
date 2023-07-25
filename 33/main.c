#include "header.h"

static char *opt_o;
static char *input_path;

static void usage(int status) {
  fprintf(stderr, "./isakacc [ -o <path> ] <file>\n");
  exit(status);
}

static void parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--help")) {
      usage(0);
    }

    if (!strcmp(argv[i], "-o")) {
      if (!argv[++i])
        usage(1);
      opt_o = argv[i];
      continue;
    }

    if (!strncmp(argv[i], "-o", 2)) {
      opt_o = argv[i] + 2;
      continue;
    }

    if (argv[i][0] == '-' && argv[i][1] != '\0')
      error("invalid option");

    input_path = argv[i];
  }

  if (!input_path)
    error("error input_path");
}

static FILE *open_file(char *path) {
  if (!path || !strcmp(path, "-"))
    return stdout;

  FILE *out = fopen(path, "w");
  if (!out)
    error("invalid open file %s: %s", path, strerror(errno));
  return out;
}

int main(int argc, char **argv) {
  parse_args(argc, argv);

  Token *tok = tokenize_file(input_path);

  // Obj *prog = parse(tok);
  FILE *out = open_file(opt_o);

  return 0;
}