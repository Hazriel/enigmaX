#include "options.h"

int main(int argc, char *argv[])
{
  struct options opts;
  if (get_user_options(&opts, argc, argv) != 0) {
    return -1;
  }
  return 0;
}
