#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "options.h"

#define _GNU_SOURCE

static const struct option options[] = {
  { "simple", no_argument, 0, 's' },
  { "inverted", no_argument, 0, 'i' },
  { "normalized", no_argument, 0, 'n' },
  { "destroy", no_argument, 0, 'd' },
  { "force", no_argument, 0, 'f' },
  { "randomize", no_argument, 0, 'r' },
  { "full-randomize", no_argument, 0, 'R' },
  { "keyfile", required_argument, 0, 'k' },
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 }
};

static const char *options_help[] = {
  "turn the scrambler off",
  "invert the coding/decoding process",
  "normalize the size of the keyfile",
  "write on top of the source file",
  "never ask something to the user after entering password",
  "randomize the name of the output file, keeping extension intact",
  "randomize the name of the output file, with the extension",
  "generate the keyfile",
  "display this help message"
};

static int is_options_end(size_t i) {
  size_t end = 0;
  while (!(options[end].name == 0 && options[end].has_arg == 0
        && options[end].flag == 0 && options[end].val == 0)) {
    ++end;
  }
  return i >= end;
}

static void print_option(const char *name, char shortname, const char *help) {
  static char buf[32] = "--";
  strcpy(&buf[2], name);
  printf(" %16s, -%c: \t%s\n", buf, shortname, help);
}

static void print_help(const char *bin) {
  printf("enigmaX - crypt or decrypt any binary data\n");
  printf("Usage: %s [options] file\n", bin);
  printf("Options:\n");
  for (size_t i = 0; !is_options_end(i); ++i) {
    print_option(options[i].name, options[i].val, options_help[i]);
  }
}

int get_user_options(struct options *opts, int argc, char *argv[]) {
  int c = 0;
  int opt_idx = 0;
  while ((c = getopt_long(argc, argv, "sindfrRk:h", options, &opt_idx)) != -1) {
    switch (c) {
      case 's':
        break;
      case 'i':
        break;
      case 'n':
        break;
      case 'd':
        break;
      case 'f':
        break;
      case 'r':
        break;
      case 'R':
        break;
      case 'k':
        break;
      case 'h':
        print_help(argv[0]);
        break;
    }
  }
  return 0;
}
