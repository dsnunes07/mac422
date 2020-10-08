#include <stdio.h>
#include <stdlib.h>

#include "race.h"
#include "cyclist.h"
#include "velodrome.h"

struct Cyclist *cyclists;
int *velodrome;

void put_cyclists_on_start_line(struct Cyclist *cyclists, int n) {
  printf("start line positions:\n");
  int groups = n/5;
  int surplus = n%5;
  // puts the full groups
  for (int i=0; i<=groups; i++) {
    printf("group %d:\n", i);
    for (int j=0; j<5*(i != groups) + surplus*(i == groups); j++) {
      set_velodrome_position(i, 2*j, cyclists[i*5 + j].id);
      printf("%s\n", cyclists[i*5 + j].name);
    }
  }
}

void configure_race(int d, int n) {
  create_velodrome(d);
  cyclists = create_cyclists(n);
  put_cyclists_on_start_line(cyclists, n);
}