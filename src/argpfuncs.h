#ifndef ARGPFUNCS_H
#define ARGPFUNCS_H

#include <argp.h>

struct arguments
{
  char *product_id, *device_id, *device_secret, *config_file_path;
};

void start_parser(int argc, char **argv, struct arguments **arguments);

#endif