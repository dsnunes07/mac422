#include <stdlib.h>
#include <stdio.h>
#include "race.h"

char* usage_msg = "usage: ./ep2 n d\n";

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("%s", usage_msg);
    exit(1);
  }

  int d = atoi(argv[1]);
  int n = atoi(argv[2]);
  configure_race(d, n);
}