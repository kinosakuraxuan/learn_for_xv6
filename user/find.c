#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

static char *
base_name(char *path)
{
  char *name;

  name = path + strlen(path);
  while(name > path && name[-1] != '/')
    name--;
  return name;
}

static void
find(char *path, char *target)
{
  char buffer[512];
  char *name;
  int fd;
  struct stat st;
  struct dirent entry;

  fd = open(path, 0);
  if(fd < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if(st.type == T_FILE || st.type == T_DEVICE){
    if(strcmp(base_name(path), target) == 0)
      printf("%s\n", path);
    close(fd);
    return;
  }

  if(st.type != T_DIR){
    close(fd);
    return;
  }

  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buffer)){
    fprintf(2, "find: path too long %s\n", path);
    close(fd);
    return;
  }

  strcpy(buffer, path);
  name = buffer + strlen(buffer);
  if(name > buffer && name[-1] != '/')
    *name++ = '/';

  while(read(fd, &entry, sizeof(entry)) == sizeof(entry)){
    if(entry.inum == 0)
      continue;

    memmove(name, entry.name, DIRSIZ);
    name[DIRSIZ] = 0;

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    find(buffer, target);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "usage: find path name\n");
    exit(1);
  }

  find(argv[1], argv[2]);
  exit(0);
}
