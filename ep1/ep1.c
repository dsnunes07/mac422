#include <stdio.h>

int scheduler;
char* trace_filename;
char* output_filename;
int print_events = 0;

void read_args(int argc, char** argv) {
  if (argc < 4) {
    printf("usage: ./ep1 scheduler trace_filename output_filename [-d]\n");
    exit(-1);
  }

  scheduler = atoi(argv[1]);
  trace_filename = argv[2];
  output_filename = argv[3];

  if (argc > 4 && strcmp(argv[4], "-d") == 0) {
    print_events = 1;
  }


}

int main(int argc, char** argv) {

  read_args(argc, argv);
  printf("input: %d %s %s\n", scheduler, trace_filename, output_filename);
  if (print_events) {
    printf("and I will print the events ;)\n");
  }
  return 0;
}