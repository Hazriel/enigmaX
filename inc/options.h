#ifndef OPTIONS_H
#define OPTIONS_H

struct options {
  int simple;
  int inverted;
  int normalized;
  int destroy;
  int force;
  int randomize;
  int randomize_full;
  int gen_keyfile;
  char *file_name;
  char *key_file;
};

/**
 * Get all the options passed in the command line and check whether these
 * options are correct.
 * \param opts a pointer to the struct options to be filled.
 * \param argc
 * \param argv
 * \return 0 if no error occured, -1 otherwise.
 */
int get_user_options(struct options *opts, int argc, char *argv[]);

/**
 * Check whether the options provided in the struct are valid options.
 * \return 0 if everything is correct, -1 otherwise.
 */
int check_user_options(struct options *opts);

/**
 * This function is for debuging purpose. It dumps all the options of the struct
 * on stdout.
 * \param opts
 */
void dump_options(struct options *opts);

#endif /* ifndef OPTIONS_H */
