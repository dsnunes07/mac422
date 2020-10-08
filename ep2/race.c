#include <stdio.h>
#include <stdlib.h>

#include "race.h"
#include "cyclist.h"
#include "velodrome.h"

void configure_race(int d, int n) {
  create_velodrome(d);
  struct Cyclist *cyclists = create_cyclists(n);
  for (int i=0; i < n; i++) {
    print_cyclist_data(&(cyclists[i]));
  }
}