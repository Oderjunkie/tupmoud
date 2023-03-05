#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "autobuild.h"

#define YES 1
#define NO 0

int fexist(const char *dir, const char *name) {
  FILE *file;
  char *path;
  path = malloc(strlen(dir) + strlen(name) + 2);
  sprintf(path, "%s/%s", dir, name);
  file = fopen(path, "r");
  free(path);
  if (file) {
    fclose(file);
    return YES;
  } else {
    return NO;
  }
}

int autbld(const char *dir) {
  if (!system(NULL)) {
    fprintf(stderr, "error: commands are disabled!\n");
    exit(EXIT_FAILURE);
  }
#define system(cmd) (printf("%s\n", cmd), system(cmd))
  if (fexist(dir, "CMakeLists.txt")) {
    char *cmds;
    cmds = malloc(strlen(dir) + sizeof("cd  && mkdir build && cd build && cmake .. && sudo cmake --build . --target install") + 1);
    sprintf(cmds, "cd %s && mkdir build && cd build && cmake .. && sudo cmake --build . --target install", dir);
    if (!system(cmds)) {
      free(cmds);
      return 0;
    }
    free(cmds);
  }
  if (fexist(dir, "configure.sh")) {
    char *cmds;
    cmds = malloc(strlen(dir) + sizeof("cd  && chmod +x ./configure.sh && ./configure.sh && make && sudo make install") + 1);
    sprintf(cmds, "cd %s && chmod +x ./configure.sh && ./configure.sh && make && sudo make install", dir);
    if (!system(cmds)) {
      free(cmds);
      return 0;
    }
    free(cmds);
  }
  if (fexist(dir, "Makefile") || fexist(dir, "makefile")) {
    char *cmds;
    cmds = malloc(strlen(dir) + sizeof("cd  && make && sudo make install") + 1);
    sprintf(cmds, "cd %s && make && sudo make install", dir);
    if (!system(cmds)) {
      free(cmds);
      return 0;
    }
    free(cmds);
  }
  if (fexist(dir, "README.md") || fexist(dir, "README.MD")) {
    FILE *file;
    char *path, buf[80], *cmds;
    int guide, block;
    path = malloc(strlen(dir) + sizeof("README.md") + 2);
    sprintf(path, "%s/%s", dir, "README.md");
    file = fopen(path, "r");
    if (!file) {
      sprintf(path, "%s/%s", dir, "README.MD");
      file = fopen(path, "r");
    }
    free(path);
    guide = block = NO;
    cmds = malloc(strlen(dir) + sizeof("cd ") + 1);
    sprintf(cmds, "cd %s", dir);
    while (!feof(file)) {
      if (!fgets(buf, sizeof(buf), file))
        break;
      if (buf[0] == '#')
        guide = !!strstr(buf, "Build") || !!strstr(buf, "Installation");
      if (strstr(buf, "```"))
        block = !block;
      if (guide && block) {
        char *cmd, *newcmds;
        cmd = strpbrk(buf, "abcdefghijklmnopqrstuvwxyz./");
        if (!cmd)
          continue;
        newcmds = malloc(strlen(cmds) + sizeof(" && ") + strlen(cmd) + 1);
        sprintf(newcmds, "%s && %s", cmds, cmd);
        newcmds[strlen(newcmds) - 1] = '\0';
        free(cmds);
        cmds = newcmds;
      }
    }
    fclose(file);
    if (!system(cmds)) {
      free(cmds);
      return -1;
    }
    free(cmds);
    return 0;
  }
  return -1;
}
