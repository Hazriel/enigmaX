#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

static int is_options_end(size_t i)
{
  size_t end = 0;
  while (!(options[end].name == 0 && options[end].has_arg == 0
        && options[end].flag == 0 && options[end].val == 0))
    ++end;
  return i >= end;
}

static void print_option(const char *name, char shortname, const char *help)
{
  static char buf[32] = "--";
  strcpy(&buf[2], name);
  printf(" %16s, -%c: \t%s\n", buf, shortname, help);
}

static void print_help()
{
  printf("enigmaX - crypt or decrypt any binary data\n");
  printf("Usage: enigmaX [options] file [keyfile]\n");
  printf("Options:\n");
  for (size_t i = 0; !is_options_end(i); ++i)
    print_option(options[i].name, options[i].val, options_help[i]);
}

static int handle_options(struct options *opts,
                          int argc,
                          char *argv[])
{
  int c = 0;
  int opt_idx = 0;
  while ((c = getopt_long(argc, argv, "sindfrRk:h", options, &opt_idx)) != -1)
  {
    switch (c)
    {
      case 's':
        opts->simple = 1;
        break;
      case 'i':
        opts->inverted = 1;
        break;
      case 'n':
        opts->normalized = 1;
        break;
      case 'd':
        opts->destroy = 1;
        break;
      case 'f':
        opts->force = 1;
        break;
      case 'r':
        opts->randomize = 1;
        break;
      case 'R':
        opts->randomize_full = 1;
        break;
      case 'k':
        opts->gen_keyfile = 1;
        break;
      case 'h':
        print_help();
        exit(0);
        break;
      case '?':
      default:
        return -1;
    }
  }
  return 0;
}

static int handle_positionnal_arguments(struct options *opts,
                                         int argc,
                                         char *argv[])
{
  while (optind >= 1 && optind < argc)
  {
    if (opts->file_name == NULL)
      opts->file_name = argv[optind++];
    else if (opts->key_file == NULL)
      opts->key_file = argv[optind++];
    else
    {
      warnx("too many arguments given.");
      return -1;
    }
  }
  return 0;
}

int get_user_options(struct options *opts, int argc, char *argv[])
{
  if (handle_options(opts, argc, argv) < 0)
    return -1;
  return handle_positionnal_arguments(opts, argc, argv);
}

int check_user_options(struct options *opts)
{
  struct stat;
  if (opts->file_name != NULL)
  {
    // FIXME: check if the file exists
  }
  else
  {
    warnx("missing file name");
    return -1;
  }
  if (opts->key_file != NULL)
  {
    // FIXME: check if the file exists
  }
  return 0;
}

void dump_options(struct options *opts)
{
  printf("Options:\n");
  printf("\t%s = %d\n", "simple", opts->simple);
  printf("\t%s = %d\n", "inverted", opts->inverted);
  printf("\t%s = %d\n", "normalized", opts->normalized);
  printf("\t%s = %d\n", "destroy", opts->destroy);
  printf("\t%s = %d\n", "force", opts->force);
  printf("\t%s = %d\n", "randomize", opts->randomize);
  printf("\t%s = %d\n", "randomize_full", opts->randomize_full);
  printf("\t%s = %d\n", "gen_keyfile", opts->gen_keyfile);
  printf("\t%s = \"%s\"\n", "file_name", opts->file_name);
  printf("\t%s = \"%s\"\n", "key_file", opts->key_file);
}
