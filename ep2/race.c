#include <stdio.h>
#include <stdlib.h>

#define race_IMPORT
#include "race.h"

#define cyclist_IMPORT
#include "cyclist.h"


void configure_race(int d, int n) {
  // create_velodrome(d);
  struct Cyclist *cyclists = create_cyclists(n);
  for (int i=0; i < n; i++) {
    print_cyclist_data(&(cyclists[i]));
  }
}