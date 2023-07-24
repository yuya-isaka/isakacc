#include "header.h"

static char *input_path;

static char *opt_o;

static void usage(int status) {
  fprintf(stderr, "./isakacc [ -o <path> ] <file>\n");
  exit(status);
}

static void parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {

    // --help
    if (!strcmp(argv[i], "--help"))
      usage(0);

    // -o パターン１
    if (!strcmp(argv[i], "-o")) {
      if (!argv[++i])
        usage(1);
      opt_o = argv[i];
      continue;
    }

    // -o パターン２
    if (!strncmp(argv[i], "-o", 2)) {
      opt_o = argv[i] + 2;
      continue;
    }

    // 他はエラー
    if (argv[i][0] == '-' && argv[i][1] != '\0')
      error("unknown argument: %s", argv[i]);

    // 入力パス（標準入力 or ファイル）
    input_path = argv[i];
  }

  if (!input_path)
    error("no input files");
}

static FILE *open_file(char *path) {
  if (!path || strcmp(path, "-") == 0)
    return stdout;

  FILE *out = fopen(path, "w");
  if (!out)
    error("cannot open output file %s: %s", path, strerror(errno));
  return out;
}

int main(int argc, char **argv) {
  parse_args(argc, argv);

  Token *tok = tokenize_file(input_path);

  Obj *prog = parse(tok);
  FILE *out = open_file(opt_o);

  codegen(prog, out);

  return 0;
}