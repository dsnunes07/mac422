#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "race.h"

char* usage_msg = "usage: ./ep2 n d [-d]\n[-d] for debug";

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("%s", usage_msg);
    exit(1);
  }

  int d = atoi(argv[1]);
  int n = atoi(argv[2]);
  int debug = 0;
  if (argc == 4 && strcmp(argv[3], "-d") == 0) {
    debug = !debug;
  }
  configure_race(d, n, debug);
}