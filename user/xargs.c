#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define LINE_SIZE 512

static void
run_command(int argc, char *argv[], char *line)
{
  char *command_args[MAXARG];
  char *cursor;
  int argument_count;
  int pid;

  argument_count = 0;
  for(int i = 1; i < argc; i++){
    if(argument_count >= MAXARG - 1){
      fprintf(2, "xargs: too many arguments\n");
      exit(1);
    }
    command_args[argument_count++] = argv[i];
  }

  cursor = line;
  while(*cursor != 0){
    while(*cursor == ' ' || *cursor == '\t' || *cursor == '\r')
      cursor++;
    if(*cursor == 0)
      break;

    if(argument_count >= MAXARG - 1){
      fprintf(2, "xargs: too many arguments\n");
      exit(1);
    }
    command_args[argument_count++] = cursor;

    while(*cursor != 0 && *cursor != ' ' && *cursor != '\t' &&
          *cursor != '\r')
      cursor++;
    if(*cursor != 0)
      *cursor++ = 0;
  }

  if(argument_count == argc - 1)
    return;

  command_args[argument_count] = 0;
  pid = fork();
  if(pid < 0){
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    exec(command_args[0], command_args);
    fprintf(2, "xargs: exec %s failed\n", command_args[0]);
    exit(1);
  }

  wait(0);
}

int
main(int argc, char *argv[])
{
  char line[LINE_SIZE];
  char character;
  int length;
  int bytes_read;

  if(argc < 2){
    fprintf(2, "usage: xargs command [arguments ...]\n");
    exit(1);
  }

  length = 0;
  while((bytes_read = read(0, &character, 1)) == 1){
    if(character == '\n'){
      line[length] = 0;
      run_command(argc, argv, line);
      length = 0;
      continue;
    }

    if(length >= LINE_SIZE - 1){
      fprintf(2, "xargs: input line too long\n");
      exit(1);
    }
    line[length++] = character;
  }

  if(bytes_read < 0){
    fprintf(2, "xargs: read failed\n");
    exit(1);
  }

  if(length > 0){
    line[length] = 0;
    run_command(argc, argv, line);
  }

  exit(0);
}
