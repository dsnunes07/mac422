#ifndef velodrome_H
#define velodrome_H

#include <stdio.h>

#ifdef velodrome_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

#define LANES 10

EXTERN void create_velodrome(int d);

#endif