#include <stdio.h>
#include <string.h>
#include "build.h"
#include "install.h"

int main(int argc, char *argv[]) {
	if (argc == 1)
    return(build());
  else
    return(insall(argc - 1, argv + 1));
}
