#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "argpfuncs.h"

#include <argp.h>

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static void process_config_file(struct arguments **arguments, struct argp_state *state);
static int check_final_values(struct arguments *arguments);

const char *argp_program_version =
    "1.0";
const char *argp_program_bug_address =
    "/dev/null";

static char doc[] =
    "Inhibitor – a program with the ability to possess a device";

static char args_doc[] = "";

static struct argp_option options[] = {
    {"product-id", 'p', "PRODID", 0, "Product id" },
    {"device-id", 'i', "DEVID", 0, "Device id" },
    {"device-secret", 's', "DEVSEC", 0, "Device secret" },
    {"config-file", 'c', "CONF", 0, "Path to configuration file, containing product-id, device-id and device-secret. Overrides product-id, device-id and device-secret. File content needs to be comma-seperated (f.e.: 'p2k3a9923q7gt133,yh3ux75ddxbyb09w,haadzce8wkro7ezw')" },
    {"daemon", 'D', 0, 0, "Launch program as a background process"},
    { 0 }
};
static struct argp argp = { options, parse_opt, args_doc, doc };



void start_parser(int argc, char **argv, struct arguments *arguments)
{
    argp_parse(&argp, argc, argv, 0, 0, arguments);
}

static void process_config_file(struct arguments **arguments, struct argp_state *state)
{
  char buffer[150];
  FILE *config_file = NULL;
  struct arguments *args = *arguments;

  if(strcmp(args->config_file_path, "") == 0){
    printf("Error: Confiuration file path is empty\n");
    argp_usage(state);
  }

  config_file = fopen(args->config_file_path, "r");
  if(config_file == NULL){
    printf("Error: Couldn't open file for reading at given configuration file path. Used path: %s\n", args->config_file_path);
    argp_usage(state);
  }

  fscanf(config_file, "%149[^\n]", buffer);

  fclose(config_file);

  char *token = strtok(buffer, ",");
  int i = 0;
    while(i<3){
        switch(i){
          case 0:
              strcpy(args->product_id, token);
              break;
          case 1:
              strcpy(args->device_id, token);
              break;
          case 2:
              strcpy(args->device_secret, token);
              break;
    }
    token = strtok(NULL, ",");
    i++;
  }
  
}
static int check_final_values(struct arguments *arguments)
{
  if(strcmp(arguments->product_id, "") == 0 || strcmp(arguments->device_id, "") == 0 || strcmp(arguments->device_secret, "") == 0){
    return 1;
  }
  else{
    return 0;
  }
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;

  switch (key){
    case 'p':
      if(strcmp(arguments->config_file_path, "") == 0){
        strcpy(arguments->product_id, arg);
      }
      break;

    case 'i':
      if(strcmp(arguments->config_file_path, "") == 0){
        strcpy(arguments->device_id, arg);
      }
      break;

    case 's':
      if(strcmp(arguments->config_file_path, "") == 0){
        strcpy(arguments->device_secret, arg);
      }
      break;

    case 'c':
      strcpy(arguments->config_file_path, arg);
      break;

    case 'D':
      arguments->daemon = 1;
      break;

    case ARGP_KEY_ARG:
      argp_usage(state);
      break;

    case ARGP_KEY_END:
      if(strcmp(arguments->config_file_path, "") != 0){
        process_config_file(&arguments, state);
      }
      if(check_final_values(arguments) != 0){
        printf("Error: Product id, device id or device secret is empty\n");
        argp_usage(state);
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}