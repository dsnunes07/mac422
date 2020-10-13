#include <stdio.h>
#include <stdlib.h>

#include "race.h"
#include "cyclist.h"
#include "velodrome.h"
#include "utils.h"

struct Cyclist *cyclists;
int *velodrome;

void put_cyclists_on_start_line(struct Cyclist *cyclists, int n) {
  int positions[n];
  range(positions, n);
  shuffle(positions, n);
  int groups = n/5;
  for (int i=0; i<=groups; i++) {
    for (int j=0; i*5 + j < n && j < 5; j++) {
      int position = positions[i*5 + j];
      set_velodrome_position(i, 2*j, cyclists[position].id);
    }
  }
}

void configure_race(int d, int n) {
  create_velodrome(d);
  cyclists = create_cyclists(n);
  put_cyclists_on_start_line(cyclists, n);

  printf("start line positions:\n");
  for (int i=0; i <= n/5; i++)
    print_velodrome_position(i);
  
  for (int i=0; i < n; i++)
    print_cyclist_data(&(cyclists[i]));
}