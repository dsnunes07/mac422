#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 128

// user input args
int scheduler;
char* trace_filename;
char* output_filename;
int print_events = 0;

// structure representing a process data
struct Process {
  char name[30];
  int arrival_time;
  int dt;
  int deadline;
  int exec_time;
};
// linked list of processes from tracefile
struct ProcessTrace {
  struct Process *current_process;
  struct ProcessTrace *next;
};

void print_linked_list(struct ProcessTrace* trace) {
  struct ProcessTrace* last = trace;
  printf("Called print linked list");
  do {
    printf("\nProcess:\nname: %s", trace->current_process->name);
    printf("\nt0: %d", trace->current_process->arrival_time);
    printf("\ndt: %d", trace->current_process->dt);
    printf("\ndeadline: %d", trace->current_process->deadline);
    printf("\nexec_time: %d", trace->current_process->exec_time);
    last = trace;
    trace = last->next;
  } while (trace->next != NULL);
}

void append_process(struct ProcessTrace** head, char* line) {
  // extract tokens from tracefile
  char* token;
  char *process_data[4];
  int i = 0;
  while((token = strsep(&line, " ")) != NULL) {
    process_data[i++] = token;
  }
  // allocates space for structs
  struct ProcessTrace* trace = malloc(sizeof(struct ProcessTrace));
  struct Process* process = malloc(sizeof(struct Process));
  // assign members of Process with data from tokens extracted from Tracefile
  strcpy(process->name, process_data[0]);
  process->arrival_time = atoi(process_data[1]);
  process->dt = atoi(process_data[2]);
  process->deadline = atoi(process_data[3]);
  process->exec_time = 0;
  // creates the current last node of list
  trace->current_process = process;
  trace->next = NULL;
  // append last node on linked list
  if (*head->current_process == NULL) {
    *head = trace;
  } else {
    struct ProcessTrace* last = *head;
    while (last->next != NULL) {
      last = last->next;
    }
    last->next = trace;
  }
}

// reads trace file and parses it to ProcessTrace linked list
struct ProcessTrace* read_trace() {
  FILE* trace_file = fopen(trace_filename, "r");
  if (trace_file == NULL) {
    printf("Could not open this tracefile: %s\n", trace_filename);
    exit(1);
  }
  struct ProcessTrace* process_list = malloc(sizeof(struct ProcessTrace));
  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), trace_file)) {
    append_process(process_list, line);
    printf("Name of heads current process: %s\n", process_list->current_process->name);
  }
  fclose(trace_file);
  return process_list;
}

// reads user input args
void read_args(int argc, char** argv) {
  if (argc < 4) {
    printf("usage: ./ep1 scheduler trace_filename output_filename [-d]\n");
    exit(1);
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
  struct ProcessTrace* trace = read_trace();
  
  //print_linked_list(trace);
  printf("input: %d %s %s\n", scheduler, trace_filename, output_filename);
  if (print_events) {
    printf("and I will print the events ;)\n");
  }
  return 0;
}