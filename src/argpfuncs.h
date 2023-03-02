#ifndef ARGPFUNCS_H
#define ARGPFUNCS_H

struct arguments
{
  char product_id[30];
  char device_id[30];
  char device_secret[30];
  char config_file_path[1024];
  int daemon;
};

void start_parser(int argc, char **argv, struct arguments *arguments);

#endif
