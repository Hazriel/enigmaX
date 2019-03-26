#include <string.h>

#include "options.h"

int main(int argc, char *argv[])
{
  struct options opts;
  memset(&opts, 0, sizeof(struct options));

  if (get_user_options(&opts, argc, argv) != 0)
    return 1;
  if (check_user_options(&opts) != 0)
    return 1;
  dump_options(&opts);
  return 0;
}
