#ifndef OPTIONS_H
#define OPTIONS_H

struct options {
  int delete_first_file;
  int randomize_file_name;
  int force;
  char *key_file;
  char *tar_name;
  char *dir_name;
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

#endif /* ifndef OPTIONS_H */
